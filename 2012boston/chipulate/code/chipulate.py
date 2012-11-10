#!/usr/bin/env python

import sys
import ConfigParser 
import urllib2
import pprint
import cPickle as pickle


# Step 0: load cfg

def loadConfig(serverIni):
    P               = ConfigParser.RawConfigParser()
    P.optionxform   = str
    P.read(serverIni)

    CFG = {}
    for section in P.sections():
        CFG[section] = dict(P.items(section))
        pass

    for (k,v) in CFG['server'].iteritems():
        app.config[k] = v
        pass
    pass

# Step 1: get echo nest analysis

# Step 2: extract tempo, chroma sequence

# Step 3: extract top two pitches for each chroma (thresholded) (=> A, B)

# Step 4: hallucinate bass line (=> C)

# Step 5: detect/make up drum pulses (=> D)

# Step 6: render output



if __name__ == '__main__':
    loadConfig('en.ini')
    pass
