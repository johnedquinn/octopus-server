/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* Internal Declarations */
Status handle_browse_request(Request *request);
Status handle_file_request(Request *request);
Status handle_cgi_request(Request *request);
Status handle_error(Request *request, Status status);

/**
 * Handle HTTP Request.
 *
 * @param   r           HTTP Request structure
 * @return  Status of the HTTP request.
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
Status handle_request(Request * r) {

    Status result;
    struct stat buf;

    /* Parse request */
    if(parse_request(r)) //this returns 0 if it works
        return handle_error(r, HTTP_STATUS_BAD_REQUEST);

    /* Determine the request path */
    if (!(r->path = determine_request_path(r->uri))) return handle_error(r, HTTP_STATUS_NOT_FOUND);

    /*  */
    if (stat(r->path, &buf) < 0) { //if it fails
        return handle_error(r, HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }

    /* Determine what function should handle request */
    if (S_ISDIR(buf.st_mode)) {
        result = handle_browse_request(r);
    } else if (S_ISREG(buf.st_mode)) {
        if (!access(r->path, X_OK)) {
            result = handle_cgi_request(r);
        } else if (!access(r->path, R_OK)) {
            result = handle_file_request(r);
        } else {
            result = handle_error(r, HTTP_STATUS_NOT_FOUND);
        }
    } else {
        result = handle_error(r, HTTP_STATUS_NOT_FOUND);
    }

    /* Determine request path */
    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type based on file type */
    log("HTTP REQUEST STATUS: %s", http_status_string(result));

    return result;
}

/**
 * Handle browse request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP browse request.
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/
Status  handle_browse_request(Request * r) {

    struct dirent **entries;
    int n;

    /* Open a directory for reading or scanning */
    n = scandir(r->path, &entries, NULL, alphasort);

    if (n < 0){
      fprintf(stderr, " Error: unable to scan directory: %s\n", strerror(errno));
      //free(entries);
      return handle_error(r, HTTP_STATUS_NOT_FOUND);
    }
    if (entries == NULL){
      fprintf(stderr, "Error: failed to open entries: %s\n", strerror(errno));
      return handle_error(r, HTTP_STATUS_NOT_FOUND);
    }

    /* Open file */
    if (!(r->file = fdopen(r->fd, "w+"))) {
        for (int i = 0; i < n; i++) {
          free(entries[i]);
        }
        free(entries);
        return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }


    /* Print HTTP Protocol */
    fprintf(r->file, "HTTP/1.0 200 OK\n");
	  fprintf(r->file, "Content-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* For each entry in directory, emit HTML list item */
    fprintf(r->file, "<!DOCTYPE html>");
    fprintf(r->file, "<head><link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\"></head>\n");
    fprintf(r->file, "<body>");
    fprintf(r->file, "<div class=\"jumbotron\"><h1>Directory: %s</h1><p>Just take a look around. Maybe you'll find some interesting stuff.</p></div>", r->uri);
    fprintf(r->file, "<div class=\"container\"><ul class=\"list-group\">\n");
    for (int i = 1; i < n; i++) {
      fprintf(r->file, "<button type=\"button\" class=\"list-group-item list-group-item-action\"><a href=\"%s/%s\">%s</a></button>\n", streq(r->uri, "/") ? "" : r->uri, entries[i]->d_name, entries[i]->d_name);
      free(entries[i]);
    }
    fprintf(r->file, "</ul></div>\n");
    fprintf(r->file, "</body>");

    /* Flush socket, return OK */
    free(entries[0]);
    free(entries);
    fflush(r->file);
    fclose(r->file);
    return HTTP_STATUS_OK;
}

/**
 * Handle file request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/
Status  handle_file_request(Request *r) {

    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;

    /* Open file for reading */
    r->file = fdopen(r->fd, "r+");
    if(!(r->file)) {
        fprintf(stderr, "Unable to open the file for reading: %s\n", strerror(errno));
        close(r->fd);
    }
    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);
    if (mimetype == NULL){
      fprintf(stderr, "NULL mimetype returned");
      goto fail;
    }
    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(stderr, "Mimetype: %s\n", mimetype);
    fprintf(r->file, "HTTP/1.0 200 OK\n");
	  fprintf(r->file, "Content-Type: %s\n", mimetype);
    fprintf(r->file, "\r\n");

    /* Read from file and write to socket in chunks */
    char * uri_path = determine_request_path(r->uri);
    //fprintf(stderr, "This is the r->uri: %s\n", uri_path);
    fs = fopen(uri_path, "r");
    if (fs == NULL){
      free(uri_path);
      fprintf(stderr,"Unable to open the URI: %s\n", strerror(errno));
      goto fail;
    }

    while ((nread = fread(buffer, sizeof(char), BUFSIZ, fs))){
      if(fwrite( buffer , sizeof(char), nread, r->file) != nread){
        fprintf(stderr, "Some read items not written: %s\n", strerror(errno));
        fclose(fs);
        free(uri_path);
        goto fail;
      }
    }

    /* Close file, flush socket, deallocate mimetype, return OK */
    fflush(r->file);
    free(mimetype);
    fclose(fs);
    fclose(r->file);
    free(uri_path);
    return HTTP_STATUS_OK;

fail:
    /* Close file, free mimetype, return INTERNAL_SERVER_ERROR */
    free(mimetype);
    fclose(r->file);
    return HTTP_STATUS_INTERNAL_SERVER_ERROR;
}

/**
 * Handle CGI request
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/
Status  handle_cgi_request(Request * r) {


    FILE *pfs;
    char buffer[BUFSIZ];

    /* Export CGI environment variables from request:
     * http://en.wikipedia.org/wiki/Common_Gateway_Interface */

    /* Export CGI environment variables from request headers */
    if (setenv("QUERY_STRING", r->query, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("DOCUMENT_ROOT", RootPath, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("REMOTE_ADDR", r->host, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("REMOTE_PORT", r->port, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("REQUEST_METHOD", r->method, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("REQUEST_URI", r->uri, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("SCRIPT_FILENAME", r->path, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    if (setenv("SERVER_PORT", Port, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    for (struct header * head = r->headers->next; head; head = head->next) {
        if (streq("Host", head->name)) {
            if (setenv("HTTP_HOST", head->value, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
        if (streq("User-Agent", head->name)) {
            if (setenv("HTTP_USER_AGENT", head->value, true) != 0) return HTTP_STATUS_INTERNAL_SERVER_ERROR;
        }
    }



    /* Popen CGI Script */
    if (!(pfs = popen(r->path, "r"))) return HTTP_STATUS_BAD_REQUEST;

    /* Copy data from popen to socket */
    r->file = fdopen(r->fd, "w+");
    if (!r->file) return HTTP_STATUS_INTERNAL_SERVER_ERROR;

    while (fgets(buffer, BUFSIZ, pfs)) {
      fputs(buffer, r->file);
    }
    /* Close popen, flush socket, return OK */
    pclose(pfs);
    fflush(r->file);
    fclose(r->file);
    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP error request.
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/
Status handle_error(Request * r, Status status) {

    const char * status_string = http_status_string(status);

    /* Write HTTP Header */
    if (!(r->file = fdopen(r->fd, "w+"))) return status;
    fprintf(r->file, "HTTP/1.0 %s\n", status_string);
    fprintf(r->file, "Content-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* Determine what HTML File should be opened */
    char * error_path;
    switch (status) {
        case (HTTP_STATUS_BAD_REQUEST):
            error_path = determine_request_path("/html/error_400.html");
            break;
        case (HTTP_STATUS_NOT_FOUND):
            error_path = determine_request_path("/html/error_404.html");
            break;
        case (HTTP_STATUS_INTERNAL_SERVER_ERROR):
            error_path = determine_request_path("/html/error_500.html");
            break;
        default:
            error_path = determine_request_path("/html/error_404.html");
            break;
    }

    FILE * error_file = fopen(error_path, "r");
    free(error_path);
    if (error_file == NULL){
        fprintf(stderr,"Unable to open the html: %s\n", strerror(errno));
        fprintf(r->file, "<h1>WRONG TURN</h1>\n");
        fprintf(r->file, "<h2>%s</h2>\n", status_string);
        fclose(r->file);
        return status;
    }

    char buffer[BUFSIZ]; int nread;
    while ((nread = fread(buffer, sizeof(char), BUFSIZ, error_file))){
      if(fwrite(buffer , sizeof(char), nread, r->file) != nread){
        fprintf(stderr, "Some read items not written: %s\n", strerror(errno));
        fclose(error_file);
        fclose(r->file);
      }
    }

    /* Return specified status */
    fflush(r->file);
    fclose(r->file);
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
