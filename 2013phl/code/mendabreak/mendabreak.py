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


@app.route('/upload', methods=['GET', 'POST'])
def upload():
    '''upload handler'''

    mangler.process_audio(  app.config, 
                            flask.request.files, 
                            float(flask.request.form['breakiness']))
    return 'wat'



# Main block
if __name__ == '__main__':
    print 'loading ', sys.argv[1]
    loadConfig(sys.argv[1])
    run(debug=True)
#     run(host='0.0.0.0')

