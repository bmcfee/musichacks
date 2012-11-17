#!/usr/bin/env python

import sqlite3
import contextlib
import ujson as json

import flask

import ConfigParser

import rdio, en, mm

# build the app
app = flask.Flask(__name__)

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

def run(**kwargs):
    app.run(**kwargs)
    pass


# --- server functions --- #

@app.before_request
def before_request():

    # connect to the database
#     flask.g.db = sqlite3.connect(app.config['database'])

    # refresh the rdio key
    try: 
        flask.g.rdio.refresh()
    except:
        flask.g.rdio = rdio.rdio(app.config)
        pass

    # set up the searcher
    if not hasattr(flask.g, 'mm'):
        flask.g.mm = mm.mm(app.config)
        pass

    # set up the echonest bindings
    if not hasattr(flask.g, 'en'):
        flask.g.en  = en.en(app.config)
        pass

    # build the session object
    # TODO:   2012-07-19 11:46:26 by Brian McFee <bmcfee@cs.ucsd.edu>

    pass

@app.teardown_request
def teardown_request(exception):
    pass

@app.route('/')
def webIndex():
    '''
        Get the index page
    '''
    return flask.render_template('index.html')

@app.route('/rdio')
def webRdioToken():
    '''
        Get the Rdio token
    '''
    return json.encode(flask.g.rdio.getToken())

@app.route('/search', methods=['GET'])
def webSearch():
    '''
        Interface to the echo nest search
    '''
    return json.encode(flask.g.en.search(flask.request.args['q']))

@app.route('/emotionprofile', methods=['GET'])
def webEmotionProfile():
    '''
        Get the emotion profile from lyrics
    '''
    return json.encode(flask.g.mm.profile(flask.request.args['track_id']))
