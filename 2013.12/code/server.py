#!/usr/bin/env python

import flask
import ConfigParser
import sys, os

import ujson as json

import rhymer

DEBUG = True

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

@app.route('/search/<query>')
def search(query):
    return json.encode(rhymer.search(query))

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

    rhymer.init(CFG)

    port = 5000
    if os.environ.get('ENV') == 'production':
        port = 80

    run(host='0.0.0.0', port=port, debug=DEBUG)


