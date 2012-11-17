// CREATED:2011-09-19 09:00:10 by Brian McFee <bmcfee@cs.ucsd.edu>
// UI control code for the radio 

var volumeOn        = true;

var sliding         = false;

var imageList       = ['neutral/1.png'];
var lyrics          = [''];

// Handle keyboard events
$(document).keydown(function (e) {
    if ($("#search").is(":focus")) {
        return;
    }
//     console.log(e.which);
    switch (e.which) {

        case 191:   // /
            $("#search").focus();
            e.preventDefault();
            break;
    
        case 77:    // M        (mute)
            toggleVolume($("#volume"));
            break;

        case $.ui.keyCode.SPACE:
            playPauseMusic();
            e.preventDefault();
            break
    }
});

function toggleVolume(buttonNode) {

    volumeOn ^= 1;

    buttonNode
        .button("option", "icons", { primary: volumeOn ? "ui-icon-volume-on" : "ui-icon-volume-off" });

    setMute(volumeOn);
}

// Initialize control widgets and start the player
$(function() {
    $( "#playpause" )
        .button({ text: false, icons: { primary: "ui-icon-play" }, disabled: true })
        .click(playPauseMusic);

    $( "#volume" )
        .button({ text: false, icons: { primary: "ui-icon-volume-on"}, disabled: true })
        .click(function() {toggleVolume($(this));});

    $( "#toolbar" )
        .buttonset();


    $( "#search" )
        .autocomplete({
            source: function( request, response ) {
                $.ajax({
                    url:        "/search",
                    dataType:   "json",
                    data:       { q: request.term },
                    success:    response 
                });
            },
            minLength: 3,       // don't search if the length is less than 2 characters
            select: function( event, ui ) {
                if (ui.item) {
                    loadSong(ui.item);
                    loadImages(ui.item.musixmatch_id)
                    $("#search").blur();
                }
                console.log( ui.item ?
                    "Selected: [" + ui.item.rdio_id + "] " + ui.item.artist + ' - ' + ui.item.title:
                    "Nothing selected, input was " + this.value );
                }
        })
        .data( "autocomplete" )._renderItem = function( ul, item ) {
            return $( "<li></li>" )
                    .data( "item.autocomplete", item )
                    .append( "<a><b>" + item.artist + "</b><br>" + item.title + "</a>" )
                    .appendTo( ul );
        };

    initRdioPlayer();
});

function loadImages(mm_id) {
    imageList   = ['neutral/1.png'];
    lyrics      = [''];
    $.getJSON(
        '/emotionprofile',
        {track_id: mm_id},
        function (data) {
            imageList   = data.images;
            lyrics      = data.lyrics;
            console.log(imageList)
        });
}

function notify(message) {
    var D = $('<div style="text-align: center;"></div>');
        
    D.append(message);
    D.dialog({  autoOpen:       true, 
                dialogClass:    'alert', 
                hide:           'fade',
                resizable:      false });
    D.delay(1000).queue(function() {
        D.dialog("close");
    });
}

function getImagelist() {
    return imageList;
}
function getLyrics() {
    return lyrics;
}

function updateFace(img) {
    $( "#emotion-art-img" ).attr('src', '/static/i/faces/' + img);
}

function updateLyrics(txt) {
    $( "#lyrics" ).text(txt);
}
