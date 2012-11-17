import urllib2
import json
import pprint
import pyechonest, pyechonest.song

class en(object):
    def __init__(self, cfg):
        self.api_key    = cfg['echonest_api_key']
        self.root       = 'http://developer.echonest.com/api/v4'
        pyechonest.config.ECHO_NEST_API_KEY = cfg['echonest_api_key']
    
        pass

    def search(self, query, n=10):

        S = pyechonest.song.search( combined    =query, 
                                    results     =n, 
                                    buckets     =['id:rdio-US', 'id:musixmatch-WW'], 
                                    limit       =True)

        Q = []
        for s in S:
            q = {}
            q['artist'] = s.artist_name
            q['title']  = s.title

            rdio_tracks = s.get_tracks('rdio-US')
            if len(rdio_tracks) == 0:
                continue
            q['rdio_id'] = rdio_tracks[0]['foreign_id']
            q['rdio_id'] = q['rdio_id'][1+q['rdio_id'].rfind(':'):]
            mm_tracks    = s.get_tracks('musixmatch-WW')
            if len(mm_tracks) == 0:
                continue
            q['musixmatch_id'] = mm_tracks[0]['foreign_id']
            q['musixmatch_id'] = q['musixmatch_id'][1+q['musixmatch_id'].rfind(':'):]
            pprint.pprint(q)

            Q.append(q)
        return Q
