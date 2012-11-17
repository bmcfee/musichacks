#!/usr/bin/env python

import urllib2
import ujson as json
import pprint


class mm(object):
    def __init__(self, cfg):
        self.api_key    = cfg['musixmatch_api_key']
        self.url        = 'http://api.musixmatch.com/ws/1.1/track.lyrics.get?format=json&apikey=%s&track_id=' % self.api_key
        pass

    def profile(self, track_id):
        f = urllib2.urlopen(self.url + track_id)
        A = json.loads(f.read())

        lyrics = A['message']['body']['lyrics']['lyrics_body']
        lines = lyrics.split('\n')

        return lines
