
$("#form").submit(function(event) { get_data(); event.preventDefault();});
$("#fight").on('click', get_data);

function get_data() {
    var artist1 = $('#artist1').val();
    var artist2 = $('#artist2').val();

    console.log('Artist 1: ' + artist1);
    console.log('Artist 2: ' + artist2);

    /* Disable inputs, set up the spinner */

    /* Execute the query */

    console.log('waiting for analysis... ');
    $.ajax({
        url: "/query/" + artist1 + "/" + artist2,
        dataType: "json"
    }).done(function(results) {
        console.log(results);
    });
});
