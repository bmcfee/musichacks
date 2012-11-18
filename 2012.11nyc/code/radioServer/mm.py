#!/usr/bin/env python

import urllib2
import ujson as json
import pprint
import numpy
import string


class mm(object):
    def __init__(self, cfg):
        # Configure musixmatch
        self.api_key    = cfg['musixmatch_api_key']
        self.metric_key = cfg['musicmetric_api_key']

        self.lyrics_url     = 'http://api.musixmatch.com/ws/1.1/track.lyrics.get?format=json&apikey=%s&track_id=' % self.api_key
        self.sentiment_url  = 'http://api.semetric.com/sentiment?token=%s&text=' % self.metric_key

        # Load in the valence-arousal model
#         self.loadModel(cfg['language_model'])
        pass

    def profile(self, track_id):
        try:
            f           = urllib2.urlopen(self.lyrics_url + str(track_id))
            A           = json.loads(f.read())

            lyrics      = A['message']['body']['lyrics']['lyrics_body']
            if len(lyrics) == 0:
                raise Exception('')

            lines       = filter(lambda x: len(x) > 0, lyrics.split('\n'))
            sentiment   = map(self.analyzeSentiment, lines)
            pprint.pprint(zip(lines, sentiment))
            # Classify sentiment
            images      = map(self.classifySentiment, sentiment)
        except:
            lines = ['']
            sentiment = ['']
            images = ['neutral/1.png']
            pass

        return {'lyrics': lines, 'sentiment': sentiment, 'images': images}


    def classifySentiment(self, sentiment):
        IMG = ['angry/2.png', 'angry/1.png', 'neutral/1.png', 'happy/1.png', 'happy/2.png']
        return IMG[int(sentiment)-1]

    def __classifySentiment(self, sentiment):
        
        # Convert to polar
        v   = sentiment[0]# + sentiment[1] * (1j)
        if numpy.isnan(v):
            v = 0.0
            pass

        # norm
        A   = numpy.abs(v)
        # Angle
        T   = numpy.angle(v, deg=True)

        # Mood map
        mood = int(numpy.floor(T / 30))

        # Emotions
        Emotions = ['pleased',  'happy',    'excited', 
                    'annoying', 'angry',    'nervous', 
                    'sad',      'bored',    'sleepy',
                    'calm',     'peaceful', 'relaxed']

        # Excitation thresholds 1/3, 2/3
        filename    = '1.png'
        dirname     = Emotions[mood] 
        if A < 1.0/3:
            dirname     = 'neutral'
        elif A > 2.0 / 3:
            filename    = '2.png'
            pass

        return '%s/%s' % (dirname, filename)

    def analyzeSentiment(self, line):
        f           = urllib2.urlopen(self.sentiment_url + urllib2.quote(line))
        A           = json.loads(f.read())

        return A['response'][0]['score']

    def __analyzeSentiment(self, line):

        words = line.split(' ')
        exclude = set(string.punctuation)
        def __cleanWord(x):
            return ''.join(ch for ch in x if ch not in exclude)
        words = filter(lambda x: x in self.model, map(__cleanWord, words))
        
        score = numpy.zeros(3)
        for w in words:
            score += self.model[w] / len(words)
            pass

        return list(score)

    def loadModel(self, infile):
        self.model = {}
        n   = 0
        mean    = numpy.zeros(3)
        mrange  = numpy.zeros(3)

        with open(infile, 'r') as f:
            for line in f:
                (word, V, A, D)     = line.strip().upper().split()
                self.model[word]    = numpy.array(map(float, [V, A, D]))
                mean                += self.model[word]
                n                   += 1
                mrange              = numpy.maximum(mrange, numpy.abs(self.model[word]))
                pass
            pass

        mean = mean / n

        for word in self.model.keys():
            self.model[word] = (self.model[word] - mean) / mrange
            pass

        for cuss in ['SHIT', 'PISS', 'FUCK', 'CUNT', 'COCKSUCKER', 'MOTHERFUCKER', 'TITS']:
            self.model[cuss] = numpy.array([-1, 1, 1])
            pass
        pass
