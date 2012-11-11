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

import numpy


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

    CFG = CFG['chipulate']

    pyechonest.config.ECHO_NEST_API_KEY = CFG['echonest_api_key']

    pass


# Step 1: get echo nest analysis
def loadCachedAnalysis(artist=None, title=None):

    CACHE_FILE = CFG['cache_file']

    if len(CACHE_FILE) == 0 or not os.path.exists(CACHE_FILE):
        # Search for the song
        S = pyechonest.song.search(artist=artist, title=title, results=1, buckets=['audio_summary'])

        if len(S) == 0:
            raise Exception('Could not find %s - %s' % (artist, title))

        # Get the track analysis
        S = S[0]
        f = urllib2.urlopen(S.audio_summary['analysis_url'])
        A = json.loads(f.read())

        if len(CACHE_FILE) > 0:
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


ALPHA   = numpy.array([1.0, 2.0, 4.0, 0.5, 0.25, 0.66, 1.5, 0.33, 0.167])
LENGTH  = ['8', '16', '32', '4', '2', '8.', '16.', '4.', '2.']
def estimateDuration(med, cur):

    # score function: log(alpha * cur / med)^2
    # for alpha in:
    #       0.25    =>  half        =>  '2'   (med / 0.25 = med * 4)
    #       0.5     =>  quarter     =>  '4'   (med / 0.50 = med * 2)
    #       1.0     =>  eighth      =>  '8'   (med / 1.0  = med * 1.0)
    #       2.0     =>  sixteenth   =>  '16'  (med / 2.0  = med * 0.5)
    #       4.0     =>  32nd        =>  '32'  (med / 4.0  = med * 0.25)
    #       2/3     =>  dotted 8th  =>  '8.'  (med / 0.66 = med * 1.5)
    #       1.5     =>  dotted 16th =>  '16.' (med / 1.5  = med * 0.75)
    #       0.33    =>  dotted 4th  =>  '4.'  (med / 0.33 = med * 3.00)
    #       0.167   =>  dotted half =>  '2.'  (med / 0.167= med * 6.00)

    global ALPHA
    global LENGTH

    i = numpy.argmin(numpy.log(ALPHA * cur / med)**2)
    
    return LENGTH[i]

def quantizeVolume(loudness, scale):
    # Effective loudness range is -70 to -10 dB
    return int(16.0 * min(50, max(0, (scale * loudness + 60))) / 50.0)


def getToneMask(key, mode):
    key = int(key)
                                                    #C C# D  D# E  F  F#  G G# A  A# B
    T = 1 - (1 - float(CFG['mode_weight'])) * numpy.array([1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1])
    T = (1 + float(CFG['mode_weight'])) - T

    # If we're in minor, shift up to major
    if not mode:
        key = (key + 3) % 12
        pass

    # now we're in major, shift the C-major scale by key
    
    return numpy.roll(T, key)

def renderMML(A):
    # Step 2: extract tempo, key info

    PITCHES = ['c', 'c+', 'd', 'd+', 'e', 'f', 'f+', 'g', 'g+', 'a', 'a+', 'b']
    BPM     = int(A['track']['tempo'])
    key     = A['track']['key']
    mode    = A['track']['mode']

#     print 'KEY: %s %s' % (PITCHES[int(key)], 'Maj' if mode else 'min'),

    tone_mask = getToneMask(key, mode)
#     print tone_mask

    channel_names = ['A', 'B', 'C', 'D', 'E']
    # Initialize envelopes
    envelopes       = [ ]
    for i in range(16):
        envelopes.append([i])
        pass

    # Initialize pulse profiles
    profiles        = [ ] 
    profiles.append('l8 o4 @02 @v15')
    profiles.append('l8 o4 @01 @v10')
    profiles.append('l8 o3 q6')
    profiles.append('l8 o0 @0')

    # Step 3: extract top two pitches for each chroma (thresholded) (=> A, B)
    features    = [(z['pitches'], z['loudness_max'], z['duration']) for z in A['segments']]
    
    channels = [ [], [], [], [] ]

    volumes  = [ [], [], [], [] ]

    # Compute median beat duration
    median_beat = numpy.median([x[2] for x in features])

    lengths = []

    for (C, loudness, duration) in features[:int(CFG['max_frames'])]:
        # Does chroma energy exceed the threshold for percussion?
        PERCUSSION = sum(C) >= float(CFG['percussion_threshold'])

        # Re-weight chroma against key profile
        C = tone_mask * C

        # Extract two largest tones
        tones   = getTwoLargest(C)

        # Compute duration from segment length
        lengths.append(estimateDuration(median_beat, duration))
    
#         print '[%.2f/%.2f/%3s] Tones: (%2s,%2s) (%.2f,%.2f,%.2f) ' % (duration, median_beat, lengths[-1], PITCHES[tones[0]], PITCHES[tones[1]], C[tones[0]], C[tones[1]], sum(C))

        channels[0].append(PITCHES[tones[0]] + lengths[-1])    # A-channel, primary tone
        channels[1].append(PITCHES[tones[1]] + lengths[-1])    # B-channel, secondary tone

        # Estimate and quantize relative volumes for each tone
        volumes[0].append(quantizeVolume(loudness, C[tones[0]]))
        volumes[1].append(quantizeVolume(loudness, C[tones[1]]))

        if C[tones[1]] < CFG['chord_energy_threshold'] * C[tones[0]]:
            volumes[1][-1] = 0
            pass

        # Step 4: hallucinate the bass line (=> C)
        channels[2].append(PITCHES[tones[0]])# + lengths[-1])    # C-channel, bass-line  (one octave below A)
        
        # Step 5: detect/make up drum pulses (=> D)
        #   look at peakiness of chromagram
        #   flat chroma == drum hit
        if PERCUSSION:
            #we're a drum hit
            # activate the noise and suppress the notes
#             volumes[0][-1] = 0
            volumes[1][-1] = 0
            channels[3].append(PITCHES[0])
            volumes[3].append(quantizeVolume(loudness, 1.0))
            pass
        else:
            # otherwise, rest the noise channel
            channels[3].append('r')
            volumes[3].append(0)
            pass

        pass


    M                   = {}
    M['channel_names']  = channel_names
    M['tempo']          = BPM
    M['profiles']       = profiles
    M['envelopes']      = envelopes
    M['volumes']        = volumes
    M['channels']       = channels
    M['lengths']        = lengths
    return M


def stitch(x,y):

    for (a, b) in zip(x, y):
        yield a
        yield b
    return

# Step 6: render output
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

        for (i, (p, s, v)) in enumerate(zip(M['profiles'], M['channels'], M['volumes'])):

            if len(p) == 0:
                continue

            cname = M['channel_names'][i]

            if len(v) == 0:
                # No volume profile (triangle wave)
                f.write('%s %s\n%s %s\n\n' % (cname, p, cname, ' '.join(s)))
            else:
                # We need to alternate volume with notes
                v = ['@v%d' % x for x in v]
                shaped = ' '.join([x for x in stitch(v, s)])
                f.write('%s %s\n %s %s\n\n' % (cname, p, cname, shaped))
            pass
        pass

    pass

if __name__ == '__main__':
    loadConfig('en.ini')
    print 'Requesting analysis for ', sys.argv[2:], '...'
    A = loadCachedAnalysis(artist=sys.argv[2], title=sys.argv[3])
    print 'Generating MML...'
    M = renderMML(A)
    print 'Saving to ', sys.argv[1]
    saveMML(sys.argv[1], M)
    pass
