
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
        html.push('<div>');

        for (var i = 0; i < data.projects.length; i++) {
            if (i % 3 == 0) {
                html.push('</div>');
                html.push('<div class="row py-3"></div>');
                html.push('<div class="card-deck">');
            }
            
            html.push(`<div class="card text-center">`);
            html.push(`<img class="card-img-top" style="max-height: 5rem; max-width: 5rem;" src="${data.projects[i].imageURL}" alt="${data.projects[i].type}">`);
            html.push('<div class="card-body">');
            html.push(`<h5 class="card-title text-dark"><a class="text-dark" href="${data.projects[i].link}">${data.projects[i].name}</a></h5>`);
            html.push(`<p class="card-text">${data.projects[i].description}</p>`);
            html.push(`<p class="card-text"><small class="text-muted">${data.projects[i].type}</small></p>`);
            html.push('</div>');
            html.push('</div>');
        }

        document.getElementById('cards-area').innerHTML = html.join('');

    });
}