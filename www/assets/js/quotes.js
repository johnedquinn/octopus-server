
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

function get_quotes(url) {

    get_json(url, function(data) {

        var html = [''];
        html.push('<div class="bg-light">');

        for (var i = 0; i < data.quotes.length; i++) {
            if (i % 2 == 0) {
                html.push('</div>');
                html.push('<div class="row py-4 bg-light"></div>');
                html.push('<div class="card-deck bg-light">');
            }
            
            html.push(`<div class="card text-center my-auto">`);
            html.push('<div class="card-body text-center my-auto">');
            html.push(`<h5 class="card-title text-dark my-auto"><a class="text-dark" rel="noopener noreferrer" target="_blank">${data.quotes[i].quote}</a></h5>`);
            //html.push(`<div class="row align-self-center">`);
            //html.push(`<div class="col">`);
            //html.push(`<h5 class="text-dark my-auto mx-auto"><a class="text-dark" rel="noopener noreferrer" target="_blank">${data.quotes[i].quote}</a></h5>`);
            //html.push(`</div></div>`);
            html.push(`<br><p class="card-text">${data.quotes[i].author}</p>`);
            html.push('</div><div class="card-footer">');
            //html.push(`<img style="max-height: 5rem; max-width: 1.5rem;" src="${data.projects[i].imageURL}" alt="${data.projects[i].type}">`);
            html.push(`<p class="card-text"><small class="text-muted">${data.quotes[i].type}</small></p>`);
            html.push('</div>');
            html.push('</div>');
        }

        document.getElementById('cards-area').innerHTML = html.join('');

    });
}