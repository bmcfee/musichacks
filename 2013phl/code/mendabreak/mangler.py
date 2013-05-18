import sys
import os
import librosa
import CDL
import numpy as np
import scipy.signal

# Load in a song, downsample  it, truncate it, chop it up
def chopsong(infile, n=15):
    y, sr = librosa.load(infile, sr=1024)
    
    # Truncate it to n seconds
    if n is None:
        n = len(y) / sr
        
    y = y[:n * sr].astype(np.float32)
    
    # half-overlapping windows
    X = np.zeros( (2 * n - 1, sr), dtype=np.float32)
    
    hop = sr / 2
    
    for t in range(0, X.shape[0]):
        X[t,:] = y[t*hop:t*hop+sr]
        
    X = X.reshape( (X.shape[0], 1, -1))
    return X

def patch_reconstruct(D, A):
    X = 0.0
    
    for k in range(D.shape[0]):
        X = X + np.fft.fft(D[k]) * np.fft.fft(A[k])
        
    return np.fft.ifft(X).real
    

def recombine(D, A):
    ''' Input: 
            Dictionary D (k-by-1-by-t), 
            sequence of activations A (n-by-k-by-1-by-t)
        Output:
            recombined signals: n-by-1-by-t
    '''
    
    n, k, _, t = A.shape
    
    X = np.zeros( (n, 1, t) )
    
    for i in range(n):
        X[i,:] = patch_reconstruct(D, A[i])
    
    return X

def combine_patches(X):
    X = X.squeeze()
    n2, t = X.shape
    
    n = (n2 + 1) / 2
    
    Z = np.zeros(n * t, dtype=np.float32)
    
    window = scipy.signal.hann(t) * 2.0 / 3
    
    hop = t / 2
    
    for i in range(n2):
        Z[(i * hop): (i * hop + t)] = Z[(i * hop): (i * hop + t)] + window * X[i]
    return Z

# <codecell>

def activation_upsample(A_low, sr):
    
    n, k, _, sr_old = A_low.shape
    
    A = np.zeros( (n, k, 1, sr) ) 
    
    for i in range(n):
        for j in range(k):
            act_res = librosa.resample(A_low[i,j,:,:].squeeze(), orig_sr=sr_old, target_sr=sr)
            
            # Local-max filter act_res
            A[i,j,:,:min(sr, len(act_res))] = librosa.localmax(act_res) * act_res
            
            
    return A

# <codecell>

def break_song(Encoder, infile, n=15, D=None):
    X = chopsong(infile, n)
    A = Encoder.transform(X)
    
    if D is not None:
        # Upsample A to match D's resolution, then reconstruct
        sr   = D.shape[2]
        A    = activation_upsample(A, sr)
    else:
        sr   = Encoder.components_.shape[2]
        D    = Encoder.components_
    
    Xhat = recombine(D, A)
        
    y    = combine_patches(Xhat)

    return y, sr

# <codecell>

# Load the low-rate patches

def initialize_data(path_d_lo, path_d_hi, alpha=0.75):
    D_low   = np.load(path_d_lo)
    D_low   = D_low.reshape( (D_low.shape[0], 1, -1) ).astype(np.float32)

    D_high  = np.load(path_d_hi)
    D_high  = D_high.reshape( (D_high.shape[0], 1, -1) ).astype(np.float32)

    Encoder = CDL.ConvolutionalDictionaryLearning(  n_atoms =   D_low.shape[0], 
                                                    n_iter  =   0, 
                                                    alpha   =   alpha)
    Encoder.fit(np.ones( (1, 1, D_low.shape[2]) ) )
    Encoder.set_codebook(D_low)
    return Encoder, D_high


# yhat, sr = break_song(Encoder, '/home/bmcfee/data/CAL500/wav/michael_jackson-billie_jean.wav', n=30, D=D_high)

# <codecell>

# librosa.output.write_wav('/home/bmcfee/Desktop/mj_amen_high.wav', yhat, sr=sr)

