# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <markdowncell>

# ### Building codebook
# 
# * Create equal-length samples of drum hits
# * Downsample to 1024Hz
# * Build CDL model
# 
# ### Resynthesis
# 
# * Downsample to 1024Hz
# * Chop into frames
# * Encode frames
# 
# Baseline:
# 
# * Resynthesize audio
# 
# Fancy:
# 
# * Upsample activation vectors
# * Local-max filtering to sparsify after upsampling
# * Resynthesize at original sample rate
# 
# Last step:
# 
# * Upload to soundcloud

# <codecell>

import sys
import os
import librosa
import CDL
import numpy as np
import scipy.signal

# <headingcell level=1>

# Generate the sample dictionary

# <codecell>

# Pad out the full signal, save it off
def padBreak():
    y, sr = librosa.load('/home/bmcfee/git/musichacks/2013phl/data/winstons-amen-brother-full.wav', sr=22050)
    ypad = np.zeros(np.ceil(len(y) / float(sr)) * sr)
    ypad[:len(y)] = y
    librosa.output.write_wav('/home/bmcfee/git/musichacks/2013phl/data/amen_%d_padded.wav' % sr, ypad, sr)

# <codecell>

# Load up the full signal, generate the full-res samples, and low-res
def makeSamples(sr=None):
    
    y, sr = librosa.load('/home/bmcfee/git/musichacks/2013phl/data/amen_22050_padded.wav', sr=sr)
    # Clip if necessary
    if len(y) % sr != 0:
        ypad = np.zeros(np.ceil(len(y) / float(sr)) * sr)
        ypad[:len(y)] = y
        y = ypad
    
    samples = y.reshape( (-1, sr) )
    
    np.save('/home/bmcfee/git/musichacks/2013phl/data/basis_%d.npy' % sr, samples)

# <codecell>

padBreak()

# <codecell>

makeSamples()
makeSamples(sr=1024)

# <headingcell level=1>

# Build the encoder

# <codecell>

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
D_low   = np.load('/home/bmcfee/git/musichacks/2013phl/data/basis_1024.npy')
D_low   = D_low.reshape( (D_low.shape[0], 1, -1) ).astype(np.float32)

D_high  = np.load('/home/bmcfee/git/musichacks/2013phl/data/basis_22050.npy')
D_high  = D_high.reshape( (D_high.shape[0], 1, -1) ).astype(np.float32)

Encoder = CDL.ConvolutionalDictionaryLearning(n_atoms=D_low.shape[0], n_iter=0, alpha=0.75)
Encoder.fit(np.ones( (1, 1, D_low.shape[2]) ) )
Encoder.set_codebook(D_low)

# <codecell>

yhat, sr = break_song(Encoder, '/home/bmcfee/data/CAL500/wav/michael_jackson-billie_jean.wav', n=30, D=D_high)

# <codecell>

librosa.output.write_wav('/home/bmcfee/Desktop/mj_amen_high.wav', yhat, sr=sr)

