
// Set up the radio widget

var radioListener = {};
var player = null;

radioListener.ready = function() {
    radioListener.is_ready = true;
    player = $("#player").get(0);
}

radioListener.playingTrackChanged = function(playingTrack, sourcePosition) {
    console.log('Started playing: ' + playingTrack);
    setTimeout(function() {player.rdio_seek(30.0);}, 100);
}

$(function() {

    $.getJSON(
        '/rdio', {}, function(data) {
            if (data) {
                params = {'playbackToken': data.playbackToken,
                            'domain':   encodeURIComponent(data.domain),
                            'listener': 'radioListener'};
                swfobject.embedSWF( 'http://www.rdio.com/api/swf',
                                    'player', '1', '1', '9.0.0',
                                    'swf/expressInstall.swf',
                                    params,
                                    {allowScriptAccess: "always"});
            }
        });

});

$("#myform").submit(function(event) { get_data(); event.preventDefault();});

function get_data() {
    var artist1 = $('#artist1').val();
    var artist2 = $('#artist2').val();

    console.log('Artist 1: ' + artist1);
    console.log('Artist 2: ' + artist2);

    /* switch to the spinner tab*/
    $("#main").addClass('hide');
    $("#montage").removeClass('hide');

    if (player != null) {
        // gonna fly now
        play_track('t2222711');
//         play_track('t1161326');
    }
    /* Execute the query */

    console.log('waiting for analysis... ');
    $.ajax({
        url: "/query/" + artist1 + "/" + artist2,
        dataType: "json"
    }).done(function(results) {

        /* switch to the fight tab */

        $("#montage").addClass('hide');
        $("#thefight").removeClass('hide');

        console.log(results);
        $("#artist1-name").html(results.playlist[0][0].artist);
        $("#artist2-name").html(results.playlist[1][0].artist);

        run_the_fight(results.playlist[0], results.playlist[1]);
    });
}

function play_track(track_id) {
    if (player == null) {
        return;
    }

    player.rdio_play(track_id);
    // seek to 30 seconds in
    player.rdio_seek(30);

}

function run_the_fight(pl1, pl2) {

    var total_health_1 = 0.0;
    var total_health_2 = 0.0;

    var num_songs = pl1.length;

    for (var i = 0; i < num_songs; i++) {
        total_health_2 += pl1[i].score;
        total_health_1 += pl2[i].score;
    }

    var total_health = Math.max(total_health_1, total_health_2);
    total_health_1 = total_health;
    total_health_2 = total_health;

    var health_1 = total_health_1;
    var health_2 = total_health_2;

    console.log('Player 1: ' + total_health_1);
    console.log('Player 2: ' + total_health_2);

    function update_p1() {
        $("#health1").css('width', (100 * health_1 / total_health_1) + '%');
    }
    function update_p2() {
        $("#health2").css('width', (100 * health_2 / total_health_2) + '%');
    }

    function fight_round(round_i, player) {

        if (round_i >= num_songs) {
            return;
        }

        if (player == 0) {
            play_track(pl1[round_i].track_id);
            health_2 -= pl1[round_i].score;
            health_2 = Math.max(0, health_2);
            // add pl1[round_i].track to player 1's list
            console.log(pl1[round_i]);
            console.log('Player 2: ' + (health_2 / total_health_2));
            $("#songs-1").append($('<li></li>').html(pl1[round_i].title + '   <b class="text-danger">' + Math.round(100  *
            pl1[round_i].score / total_health_2) +'% damage!</b>'));
        } else {
            play_track(pl2[round_i].track_id);
            health_1 -= pl2[round_i].score;
            health_1 = Math.max(0, health_1);
            // add pl2[round_i].track to player 2's list
            console.log(pl2[round_i]);
            console.log('Player 1: ' + (health_1 / total_health_1));
            $("#songs-2").append($('<li></li>').html(pl2[round_i].title + '  <b class="text-danger">' + Math.round(100  *
            pl2[round_i].score /
            total_health_1) +'% damage!</b>'));
        }
        update_p1();
        if (health_1 <= 1e-10) {
            // Player 2 wins
            console.log('Player 2 wins!');
            $("#player2wins").removeClass('hide');
            $("#player1box").addClass('muted');
            return;
        }

        update_p2();
        if (health_2 <= 1e-10) {
            // Player 1 wins
            console.log('Player 1 wins!');
            $("#player1wins").removeClass('hide');
            $("#player2box").addClass('muted');
            return;
        }


        setTimeout( function() {
            fight_round(round_i + player, (player + 1) % 2);
        }, 5000);
    }

    // Start the fight
    update_p1();
    update_p2();
    fight_round(0, 0);
}

