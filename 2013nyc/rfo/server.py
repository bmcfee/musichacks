#!/usr/bin/env python

import flask
import ConfigParser
import sys, os

import ujson as json
import rdio

import rep

DEBUG = True
SECRET_KEY = 'yodawg'

# construct application object
app = flask.Flask(__name__)
app.config.from_object(__name__)

def load_config(server_ini):
    P       = ConfigParser.RawConfigParser()

    P.opionxform    = str
    P.read(server_ini)

    CFG = {}
    for section in P.sections():
        CFG[section] = dict(P.items(section))

    for (k, v) in CFG['server'].iteritems():
        app.config[k] = v
    return CFG

def run(**kwargs):
    app.run(**kwargs)

@app.before_request
def before_request():

    try: 
        flask.g.rdio.refresh()
    except:
        flask.g.rdio = rdio.rdio(CFG['rdio'])
        pass


    pass

@app.route('/rdio')
def web_rdio_token():
    return json.encode(flask.g.rdio.get_token())

@app.route('/query/<artist1>/<artist2>')
def artist_query(artist1, artist2):

    obj1 = rep.get_artist_data(CFG['echonest'], artist1)
    obj2 = rep.get_artist_data(CFG['echonest'], artist2)
    n = min(len(obj1), len(obj2))
    return json.encode({'playlist': [obj1[:n], obj2[:n]]})


@app.route('/', methods=['GET'])
def index():
    '''Top-level web page'''
    return flask.render_template('index.html')


# Main block
if __name__ == '__main__':
    if len(sys.argv) > 1:
        CFG = load_config(sys.argv[1])
    else:
        CFG = load_config('server.ini')

    port = 5000
    if os.environ.get('ENV') == 'production':
        port = 80

    run(host='0.0.0.0', port=port, debug=DEBUG)


