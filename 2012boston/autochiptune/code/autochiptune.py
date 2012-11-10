#!/usr/bin/env python

import sys, os.path
import ConfigParser 
import pprint
import cPickle as pickle
import ujson as json
import urllib2

import pyechonest
import pyechonest.song, pyechonest.track
import pyechonest.config

import heapq


# Step 0: load cfg
CFG = {}

def loadConfig(serverIni):
    global CFG
    P               = ConfigParser.RawConfigParser()
    P.optionxform   = str
    P.read(serverIni)

    CFG = {}
    for section in P.sections():
        CFG[section] = dict(P.items(section))
        pass

    pyechonest.config.ECHO_NEST_API_KEY = CFG['chipulate']['echonest_api_key']

    pass


# Step 1: get echo nest analysis
def loadCachedAnalysis(artist=None, title=None):

    CACHE_FILE = CFG['chipulate']['cache_file']

    if CACHE_FILE is None or not os.path.exists(CACHE_FILE):
        # Search for the song
        S = pyechonest.song.search(artist=artist, title=title, results=1, buckets=['audio_summary'])

        if len(S) == 0:
            raise Exception('Could not find %s - %s' % (artist, title))

        # Get the track analysis
        S = S[0]
        f = urllib2.urlopen(S.audio_summary['analysis_url'])
        A = json.loads(f.read())

        if CACHE_FILE is not None:
            with open(CACHE_FILE, 'w') as f:
                pickle.dump(A, f)
                pass
    else:
        print 'Loading cached analysis'
        with open(CACHE_FILE, 'r') as f:
            A = pickle.load(f)
            pass
        pass
    return A

def getTwoLargest(C):

    (v1, v2) = (-1, -1)
    (i1, i2) = (0, 0)

    for (i, v) in enumerate(C):
        if v > v1:
            i2 = i1
            v2 = v1
            i1 = i
            v1 = v
        elif v > v2:
            v2 = v
            i2 = i
            pass
        pass
    return (i1, i2)


def renderMML(A):
    # Step 2: extract tempo, key info

    PITCHES = ['c', 'c+', 'd', 'd+', 'e', 'f', 'f+', 'g', 'g+', 'a', 'a+', 'b']
    BPM     = int(A['track']['tempo'])
    key     = A['track']['key']
    mode    = A['track']['mode']

    channel_names = ['A', 'B', 'C', 'D', 'E']
    # Initialize envelopes
    envelopes       = [ [] ] * len(channel_names)
    envelopes[0]    = [15]
    envelopes[1]    = [10]

    # Initialize pulse profiles
    profiles        = [ [] ] * len(channel_names)
    profiles[0]     = 'l8 o4 @01 @v0'
    profiles[1]     = 'l8 o5 @02 @v1'
    profiles[2]     = 'l8 o3 q7'

    # Step 3: extract top two pitches for each chroma (thresholded) (=> A, B)
    chroma  = [z['pitches'] for z in A['segments']]
    
    channels = [ [] ] * len(channel_names)
    for C in chroma[:128]:
        tones = getTwoLargest(C)

        print 'Tones: (%2d,%2d) (%.2f,%.2f,%.2f) ' % (tones[0], tones[1], C[tones[0]], C[tones[1]], sum(C))

        channels[0].append(PITCHES[tones[0]])    # A-channel, primary tone
        channels[1].append(PITCHES[tones[1]])    # B-channel, secondary tone

        # Step 4: hallucinate bass line (=> C)
        channels[2].append(PITCHES[tones[0]])    # C-channel, bass-line  (one octave below A)
        
        pass


    # Step 5: detect/make up drum pulses (=> D)
    #   look at peakiness of chromagram
    #   flat chroma == drum hit

    # Step 6: render output



    M                   = {}
    M['channel_names']  = channel_names
    M['tempo']          = BPM
    M['profiles']       = profiles
    M['envelopes']      = envelopes
    M['channels']       = channels
    return M


def saveMML(output, M):

    with open(output, 'w') as f:
        # Store the envelopes
        for (i, s) in enumerate(M['envelopes']):
            if len(s) == 0:
                continue
            f.write('@v%d = { %s }\n' % (i, ' '.join(map(lambda x: '%d' % x, s))))
            pass

        # Set the tempo
        f.write('\n%s t%d\n\n' % (''.join(M['channel_names']), M['tempo']))

        for (i, (p, s)) in enumerate(zip(M['profiles'], M['channels'])):
            if len(p) == 0:
                continue
            f.write('%s %s\n%s %s\n\n' % (M['channel_names'][i], p, M['channel_names'][i], ' '.join(s)))
            pass
        pass

    pass

if __name__ == '__main__':
    loadConfig('en.ini')
    A = loadCachedAnalysis(artist=sys.argv[2], title=sys.argv[3])
    M = renderMML(A)
    saveMML(sys.argv[1], M)
    pass
