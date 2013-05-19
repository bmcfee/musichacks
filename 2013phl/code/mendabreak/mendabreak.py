#!/usr/bin/env python

import flask
import ConfigParser
import sys

import soundcloud

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

@app.before_request
def before_request():

    # Refresh the soundcloud client object
    flask.g.client = soundcloud.Client( client_id=CFG['soundcloud']['client_id'],
                                        client_secret=CFG['soundcloud']['client_secret'],
                                        redirect_uri='http://localhost:5000/')
    # load/construct the session object

    pass


@app.route('/', methods=['GET'])
def index():
    '''Top-level web page'''

    if 'access_token' in flask.request.cookies:
        access_token = flask.request.cookies['access_token']
    # exchange authorization code for access token
    elif 'code' in flask.request.args:
        code = flask.request.args['code']
        access_token = flask.g.client.exchange_token(code)
    else:
        return flask.redirect(flask.g.client.authorize_url())

    response = flask.make_response(flask.render_template('index.html'))
    response.set_cookie('access_token', access_token)
    return response


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
    CFG = loadConfig(sys.argv[1])
    run(debug=True)
#     run(host='0.0.0.0')

