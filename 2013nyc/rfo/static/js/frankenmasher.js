$(function() {
    get_mashes();
});

function get_mashes() {

    SC.get('/tracks/', {limit: 20, q: 'frankenmasher2000'}, 
    function(tracks) {
        var mashlist = $('#mashlist');
        
        $("#mashlist > li").remove();

        for (var i = 0; i < tracks.length ; i++) {
            var new_song = $('<li></li>')
                            .attr('id', 'frankenmash_' + i);

            mashlist.append(new_song);

            SC.oEmbed(tracks[i].permalink_url, 
                    { },
                document.getElementById(new_song.attr('id')));
        }
    });

}
