#!/usr/bin/env python

import radioServer

if __name__ == '__main__':
    radioServer.loadConfig('server.ini')
    radioServer.run(host='0.0.0.0')
    pass
