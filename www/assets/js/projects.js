
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
        html.push('<div class="bg-light">');

        for (var i = 0; i < data.projects.length; i++) {
            if (i % 3 == 0) {
                html.push('</div>');
                html.push('<div class="row py-4 bg-light"></div>');
                html.push('<div class="card-deck bg-light">');
            }
            
            html.push(`<div class="card text-center">`);
            html.push('<div class="card-body">');
            html.push(`<h5 class="card-title text-dark"><a class="text-dark" rel="noopener noreferrer" target="_blank" href="${data.projects[i].link}">${data.projects[i].name}</a></h5>`);
            html.push(`<p class="card-text">${data.projects[i].description}</p>`);
            html.push('</div><div class="card-footer">');
            html.push(`<img style="max-height: 5rem; max-width: 1.5rem;" src="${data.projects[i].imageURL}" alt="${data.projects[i].type}">`);
            html.push(`<p class="card-text"><small class="text-muted">${data.projects[i].type}</small></p>`);
            html.push('</div>');
            html.push('</div>');
        }

        document.getElementById('cards-area').innerHTML = html.join('');

    });
}