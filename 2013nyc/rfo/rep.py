# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <codecell>

import json

import urllib2

import pyen

import numpy as np
import scipy

import librosa

def __repetition_score(X, window=9):
    
    n = X.shape[1]

    k = np.ceil(np.sqrt(n))
    
    R = librosa.segment.recurrence_matrix(  X, 
                                            k=float(k)/n, 
                                            width=15, 
                                            metric='seuclidean', 
                                            sym=False)
    
    S = librosa.segment.structure_feature(R).astype(np.float32)
    S = scipy.signal.medfilt2d(S, np.array([1, window]))
    
    return np.mean(np.sum(S, axis=0))

def repetition_score(analysis):
    X = np.array([v['timbre'] for v in analysis['segments']])
    return __repetition_score(X.T)


def get_analysis(url):
    
    f = urllib2.urlopen(url)
    data = json.loads(f.read())
        
    return data

def get_artist_data(cfg, query, n=5):
    en = pyen.Pyen(api_key=cfg['echonest_api_key'])
    
    try:
        songs = en.get('playlist/static', {'artist': query,
                                           'song_type': 'studio',
                                           'bucket': ['id:rdio-US', 'tracks', 'audio_summary'],
                                           'limit': True,
                                           'results': n})['songs']
    except:
        songs = []


    response = []
    for s in songs:
        analysis = get_analysis(s['audio_summary']['analysis_url'])
        track_id = s['tracks'][0]['foreign_id'].split(':')[-1]
        value = repetition_score(analysis)

        response.append( (value, track_id, s['artist_name'], s['title']) )
    
    return sorted(response)

