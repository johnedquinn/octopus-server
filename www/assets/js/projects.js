
function get_json(url, callback) {
    var request = new XMLHttpRequest();
    request.onreadystatechange = function() {
        if (request.readyState == 4 && request.status == 200) {
            callback(JSON.parse(request.responseText));
        }
    }
    request.open('GET', url, true);
    request.send();
}

function get_projects(url) {

    get_json(url, function(data) {

        var html = [''];
        html.push('');

        //Object.keys(data).forEach(function(project) {
        for (var i = 0; i < data.projects.length; i++) {
            html.push('<div class="card">');
            html.push('<div class="card-body">');
            html.push(`<h5 class="card-title text-dark">${data.projects[i].name}</h5>`);
            html.push(`<p>${data.projects[i].description}</p>`);
            html.push('</div>');
            html.push('</div>');
        //});
        }

        document.getElementById('cards-area').innerHTML = html.join('');

    });
}