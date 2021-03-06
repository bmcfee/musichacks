$("#myform").submit(function(event) {
    get_data();
    event.preventDefault();
});

function get_data() {
    var query = $("#query").val();

    $('#noresults').addClass('hide');

    // Execute the query
    $.ajax({
        url: "/search/" + query,
        dataType: "json"
    }).done(function(results) {
        // Populate the results list
        
        $("#results > li").remove();
        if (results.length == 0) {
            // No results, not funny
            $('#noresults').removeClass('hide');
            return;
        }
        $('ul').css('column-count', Math.min(3, results.length));
        $('ul').css('-moz-column-count', Math.min(3, results.length));
        $('ul').css('-webkit-column-count', Math.min(3, results.length));
        for (var i = 0; i < results.length; i++){
            $("#results").append($('<li class="list-group-item"></li>').html(results[i]));
        }
    });
}
