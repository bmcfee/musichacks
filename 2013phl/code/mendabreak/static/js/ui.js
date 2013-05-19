// hello world

$("input:file").change( function() {
    var songname = $(this).val();

    $("#song-title").val( songname
                            .split('\\').pop()
                            .split('.', 2).shift()
                            .replace(/_/g, ' ')
                            .replace('-', ' - ') + ' [Mend-a-break remix]');


    $("button:submit").removeClass('disabled').removeAttr('disabled');
});
