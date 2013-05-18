#!/usr/bin/env python

import flask
import ConfigParser
import sys

import mangler

# construct application object
app = flask.Flask(__name__)

def loadConfig(server_ini):
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

@app.route('/')
def index():
    '''Top-level web page'''

    return flask.render_template('index.html')


@app.route('/upload')
def upload():
   '''upload handler'''
   Encoder, D_hi = mangler.initialize_data( app.config['D_lo'], 
                                            app.config['D_hi'],
                                            flask.request.args['sparsity']) 
   return []



# Main block
if __name__ == '__main__':
    print 'loading ', sys.argv[1]
    loadConfig(sys.argv[1])
    run(host='0.0.0.0')

