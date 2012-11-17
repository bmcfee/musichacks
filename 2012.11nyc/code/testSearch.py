#!/usr/bin/env python

import sys
import radioServer.en
import pyechonest, pyechonest.song, pyechonest.config
import ujson as json
import urllib2
import ConfigParser
import pprint

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

    CFG = CFG['server']

    pyechonest.config.ECHO_NEST_API_KEY = CFG['echonest_api_key']

    pass

def enSearch(query):

    S = pyechonest.song.search(combined=query, results=10, buckets=['id:rdio-US', 'id:musixmatch-WW'], limit=True)

    pprint.pprint(S)
    S = S[0]
    print S.artist_name, ' - ', S.title
    print S.get_tracks('rdio-US')[0]
    return S.get_tracks('musixmatch-WW')[0]

def mmSearch(mmid):

    url = 'http://api.musixmatch.com/ws/1.1/track.lyrics.get?format=json&apikey=%s&track_id=%s' % (CFG['musixmatch_api_key'], mmid)
    print url
    f = urllib2.urlopen(url)
    A = json.loads(f.read())

    print A['message']['body']['lyrics']['lyrics_body']
    pass

if __name__ == '__main__':
    loadConfig('server.ini')
    mm = enSearch(' '.join(sys.argv[:]))
    mm_id = mm['foreign_id']
    mmSearch(mm_id[1+mm_id.rfind(':'):])
    pass
