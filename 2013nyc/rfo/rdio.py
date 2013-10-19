import rdioapi
import time

class rdio(object):
    def __init__(self, cfg):
        self.api_key    = cfg['rdio_api_key']
        self.secret     = cfg['rdio_secret']
        self.domain     = cfg['rdio_domain']
        self.ttl        = cfg['rdio_playback_ttl']
        self.state      = {}

        self.R          = rdioapi.Rdio(self.api_key, self.secret, self.state)

        self.token      = self.R.call('getPlaybackToken', domain=self.domain)
        self.timestamp  = time.time()

        pass

    def refresh(self):
        if time.time() - self.timestamp > self.ttl:
            self.__init__(self.api_key, self.secret, self.domain, self.ttl)
        pass

    def get_token(self):
        return {'playbackToken': self.token, 'domain': self.domain}

