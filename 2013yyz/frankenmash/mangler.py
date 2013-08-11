import os
import numpy as np
import tempfile
import subprocess

import rpca
import librosa

# <codecell>

SR         = 22050
N_FFT      = 1024
HOP_LENGTH = N_FFT / 4
WIN_HPSS   = 19

# <codecell>

def rpca_correct(X, L, S, p=0):
    
    # Recombobulate into an energy partition
    L = np.maximum(0.0, L)
    L = np.minimum(X, L)
    S = X - L
    
    # Wiener-filter to smooth out the edges
    if p > 0:
        zS = S == 0
        S = S ** p
        S[zS] = 0.0
    
        zL = L == 0
        L = L ** p
        L[zL] = 0.0
    
        L[zL & zS] = 0.5
        S[zL & zS] = 0.5
    
        Ml = L / (L + S)
        Ms = S / (L + S)
    
        return (X * Ml, X * Ms)
    else:
        return (L, S)

# <codecell>

def decompose(filename, offset=0, duration=30, voice=True):
    '''Decompose a song into its pieces

    :parameters:
    - filename : str
        path to the audio
    - offset : float
        initial offset for loading audio
    - duration : float
        maximum amount of audio to load

    :returns:
    - D : np.array, dtype=complex
        STFT of the full signal
    - D_inst : np.array, dtype=complex
        STFT of the instruments
    - D_vox : np.array, dtype=complex
        STFT of the vocals
    - D_inst_harm : np.array, dtype=complex
        STFT of the instrument harmonics
    - D_inst_perc : np.array, dtype=complex
        STFT of the instruments percussives
    '''
    y, sr = librosa.load(filename, sr=SR, offset=offset, duration=duration)
    
    # Step 1: compute STFT
    D = librosa.stft(y, n_fft=N_FFT, hop_length=HOP_LENGTH).astype(np.complex64)
    
    # Step 2: separate magnitude and phase
    S, P = librosa.magphase(D)
    S    = S / S.max()
    
    if voice:
        tau = (D.shape[0] * 3) / 4
    
        # Step 3: RPCA to separate voice and background
        S1, S2, _ = rpca.robust_pca(S[:tau,:], max_iter=25)
        S1, S2    = rpca_correct(S[:tau,:], S1, S2)
    
        S1 = np.vstack((S1, S[tau:,:]))
        S2 = np.vstack((S2, S[tau:,:]))
    else:
        S1, S2 = librosa.hpss.hpss_median(S, win_H=WIN_HPSS, win_P=WIN_HPSS, p=1.0)
    
    # Step 4: recombine with phase
    return D, S1 * P, S2 * P

# <codecell>

def onsets(D):
    S = librosa.logamplitude(D)
    o = np.diff(S, axis=1)
    o = np.maximum(0, o)
    o = np.median(o, axis=0)
    o = o / o.max()
    return o

# <codecell>

def beat_stretch(D, beats_orig, beats_target):
    
    D_out = np.empty( (D.shape[0], beats_target[-1]), dtype=D.dtype)
    
    # Compute beat deltas
    
    db_orig   = np.diff(beats_orig)
    db_target = np.diff(beats_target)
    
    t = 0

    for (i, delta) in enumerate(db_target):
        Dslice = D[:, beats_orig[i]:beats_orig[i+1]]
        
        Dnew   = librosa.phase_vocoder(Dslice, float(db_orig[i])/delta)
        
        D_out[:, t:t+delta] = Dnew[:,:delta]
        
        t = t + delta
        
    return D_out

# <codecell>

def remix(song1, song2, offset=0, duration=45, alpha=0.75,voice=True):
    
    # Load the first song
    D1, Di1, Dv1 = decompose(song1, offset=offset, duration=duration, voice=voice)

    # Load the second song
    D2, Di2, Dv2 = decompose(song2, offset=offset, duration=duration+15, voice=voice)
    
    # Track beats in song 1
    tempo1, beats1 = librosa.beat.beat_track(sr=SR, onsets=onsets(D1), hop_length=HOP_LENGTH, n_fft=N_FFT, trim=False)
    
    # Track beats in song 2
    tempo2, beats2 = librosa.beat.beat_track(sr=SR, onsets=onsets(D2), hop_length=HOP_LENGTH, n_fft=N_FFT, trim=False, start_bpm=tempo1)

    # If we don't have enough beats, subsample
    if len(beats2) < len(beats1):
        beats1 = beats1[::len(beats2)]
    
    # Pad the beat boundaries out
    pad_beats1 = np.unique(np.hstack((0, beats1, D1.shape[1])))
    pad_beats2 = np.unique(np.hstack((0, beats2, D2.shape[1])))
    
    # If len(beats2) > len(beats1), we're fine...
    # Otherwise, we need to combine beats1 down to len(beats2)
    Dh_stretch = beat_stretch(Di2, pad_beats2, pad_beats1)

    D_mix_1 = Dv1
    D_mix_1 = D_mix_1 / np.abs(D_mix_1).max()
    
    D_mix_2 = Dh_stretch
    D_mix_2 = D_mix_2 / np.abs(D_mix_2).max()
    
    y_out = librosa.istft(alpha * D_mix_1 + (1.0 - alpha) * D_mix_2, 
                            n_fft=N_FFT, hop_length=HOP_LENGTH)

    return y_out, SR


def encode_mp3(wavfile):
    
    code, mp3file = tempfile.mkstemp(suffix='.mp3')

    subprocess.call(['avconv', '-y', '-i', wavfile, '-acodec', 'libmp3lame', mp3file])

    return mp3file

def process_audio(cfg, files, title, alpha, client):

    # Build a temp file for the upload tracks
    code_a, tmp_ul_a = tempfile.mkstemp()
    files['song_A'].save(tmp_ul_a)

    # Build a temp file for the upload track
    code_b, tmp_ul_b = tempfile.mkstemp()
    files['song_B'].save(tmp_ul_b)

    # Encode

    yhat, sr = remix(tmp_ul_a, tmp_ul_b, duration=int(cfg['max_time']), alpha=alpha)
    
    # Delete the upload
    os.unlink(tmp_ul_a)
    # Delete the upload
    os.unlink(tmp_ul_b)

    # Save the output
    code, tmp_out = tempfile.mkstemp(suffix='.wav')
    librosa.output.write_wav(tmp_out, yhat, sr=sr)

    # Encode the mp3
    mp3file = encode_mp3(tmp_out)
    
    # Delete the wavfile
    os.unlink(tmp_out)

    track = client.post('/tracks', track={
            'title': title,
            'created_wth': 'Frankenmasher 2000',
            'tag_list': 'frankenmasher2000',
            'asset_data': open(mp3file, 'rb')
    })
    os.unlink(mp3file)
    
    return track.permalink_url
