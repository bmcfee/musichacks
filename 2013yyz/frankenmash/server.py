#!/usr/bin/env python

import flask
import ConfigParser
import sys, os

import soundcloud

import mangler

DEBUG = True
SECRET_KEY = 'yodawg'

# construct application object
app = flask.Flask(__name__)
app.config.from_object(__name__)

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
                                        redirect_uri=CFG['soundcloud']['redirect_uri'])


@app.route('/', methods=['GET'])
def index():
    '''Top-level web page'''

    if 'code' in flask.request.args:
        code = flask.request.args['code']
        flask.session['code']         = code
        flask.session['access_token'] = flask.g.client.exchange_token(code)

    if 'access_token' not in flask.session:
        return flask.redirect(flask.g.client.authorize_url())

    return flask.make_response(flask.render_template('index.html', client_id=CFG['soundcloud']['client_id']))


@app.route('/upload', methods=['GET', 'POST'])
def upload():
    '''upload handler'''

    try:
        client = soundcloud.Client(access_token=flask.session['access_token'].access_token)
    except KeyError:
        return flask.redirect(flask.url_for('index'))
    
    url = mangler.process_audio(    app.config, 
                                    flask.request.files, 
                                    flask.request.form['title'],
                                    float(flask.request.form['alpha']),
                                    client)
    return flask.redirect(url)

# Main block
if __name__ == '__main__':
    if len(sys.argv) > 1:
        CFG = loadConfig(sys.argv[1])
    else:
        CFG = loadConfig('server.ini')

    port = 5000
    if os.environ.get('ENV') == 'production':
        port = 80

    run(host='0.0.0.0', port=port, debug=DEBUG)


