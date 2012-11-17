var radioListener = {};
var player = null;
var paused = true;

var trackDuration       = 0;

function playPauseMusic() {
    if (player == null) {
        return;
    }

    if (paused) {
        paused = false;
        player.rdio_play();
    } else {
        paused = true;
        player.rdio_pause();
    }
}

function stopMusic() {
    if (player != null) {
        player.rdio_stop();
    }
    resetPlayerDisplay();
}

function resetPlayer() {
    stopMusic();

    $("#playpause") .button("option", "disabled", true);

    $("#trackprogress")
        .slider("option", "disabled", true)
        .slider("option", "value", 0);

}

function resetPlayerDisplay() {
    $("#song-title")    .text('');
    $("#artist-name")   .text('');
    $("#album-title")   .text('');
    $("#album-art-img") .attr('src', '/static/i/logo.png');
    $("#artist-info").fadeOut('fast', function() {
        $("#artist-info").html('')
    });
}

function seekTrack() {
    var progress = $("#trackprogress");
    if (player != null) {
        offset = Math.round(progress.slider("option", "value") * trackDuration / 100);
        player.rdio_seek(offset);
        progress.blur();
    }
}

function setMute(volume) {
    if (player != null) {
        player.rdio_setMute(! volume);
    }
}

radioListener.ready = function() {
    radioListener.is_ready = true;
    $("#volume")
        .button("option", "disabled", false);

    player = $("#player").get(0);
    console.log('Ready to play.');
}

radioListener.playStateChanged = function(playState) {
    //  The playback state has changed. 
    //  The state can be: 0 → paused, 1 → playing, 2 → stopped, 3 → buffering or 4 → paused.
    states = ['Paused', 'Playing', 'Stopped', 'Buffering', 'Paused']
    console.log('play state: ' + states[playState]);
    if (playState == 0 || playState == 4) {
        paused = true;
    } else {
        paused = false;
    }
    if (playState == 1) {
        $( "#playpause" ).button("option", "icons", {primary: 'ui-icon-pause'});
    } else {
        $( "#playpause" ).button("option", "icons", {primary: 'ui-icon-play'});
    }

    if (playState != 2) {
        $("#trackprogress").slider("option", "disabled", false);
    }

}


radioListener.playingTrackChanged = function(playingTrack, sourcePosition) {
    //  The currently playing track has changed. 
    //  Track metadata is provided as playingTrack and the position within the playing source as sourcePosition.

    if (playingTrack == null) {
        // we're all out of tracks
        resetPlayerDisplay();
        
        return;
    } else if ( $("li.playing").next().length == 0 ) {
        // we're on the last track, but in radio mode.. add another
        expandPlaylist();
    }
    console.log('Playing: ' + playingTrack.artist + ' - ' + playingTrack.name );

    trackDuration = playingTrack.duration;

    $("#song-title")
        .text(playingTrack.name);

    $("#artist-name")
        .text(playingTrack.artist);

    $("#album-title")
        .text(playingTrack.album);

    $("#album-art-img")
        .attr('src', playingTrack.icon);
}

radioListener.playingSourceChanged = function(playingSource) {
    //  The currently playing source changed. 
    //  The source metadata, including a track listing is inside playingSource.
}

radioListener.volumeChanged = function(volume) {
    //  The volume changed to volume, a number between 0 and 1.
    console.log('Volume: ' + volume);
}

radioListener.muteChanged = function(mute) {
    //  Mute was changed. 
    //  mute will either be true (for muting enabled) or false (for muting disabled).
    console.log('Mute: ' + mute);
}

radioListener.positionChanged = function(position) {
    //  The position within the track changed to position seconds. 
    //  This happens both in response to a seek and during playback.

    if (! sliding) {
        $("#trackprogress").slider({value: Math.round(position * 100 / trackDuration)});
    }
}

radioListener.queueChanged = function(newQueue) {
    //  The queue has changed to newQueue.
    console.log('Queue: ' + JSON.stringify(newQueue));
}

radioListener.shuffleChanged = function(shuffle) {
    //  The shuffle mode has changed. 
    //  shuffle is a boolean, true for shuffle, false for normal playback order.
}

radioListener.repeatChanged = function(repeatMode) {
    //  The repeat mode change. 
    //  repeatMode will be one of: 0 → no-repeat, 1 → track-repeat or 2 → whole-source-repeat.
}

radioListener.updateFrequencyData = function(arrayAsString) {
    //  Receive a frequency update. 
    //  The data comes as a string that is a comma separated array of floats in the range 0-1.
}

radioListener.playingSomewhereElse = function() {
    //  An Rdio user can only play from one location at a time. 
    //  If playback begins somewhere else then playback will stop and radioListener callback will be called.
    console.log("CAN'T PLAY HERE");
}


function enablePlaylistWidgets() {
    $("#playpause")
        .button("option", "disabled", false);
}

function loadSong(item) {
    console.log(item);
    $("#playpause").button("option", "disabled", false);
    player.rdio_play(item.rdio_id);
}

function initRdioPlayer() {
    
    resetPlayer();

    $.getJSON(
        '/rdio', 
        {}, 
        function(data) {
            if (data) {
                params = {  'playbackToken':    data.playbackToken,
                            'domain':           encodeURIComponent(data.domain),
                            'listener':         'radioListener' };

                swfobject.embedSWF(     'http://www.rdio.com/api/swf',  // embed url
                                        'player',                       // element id to replace
                                        '1', '1',                       // width and height
                                        '9.0.0',                        // flash version
                                        'swf/expressInstall.swf',       // url to express install swf
                                        params, 
                                        {allowScriptAccess: "always"});

            }
        });
}
