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
    /* so result isn't getting set to anything really. I'm not confident in much of this it's just an idea -Emma */
    Status result;
    struct stat buf;

    /* Parse request */
    if(parse_request(r)); //this returns 0 if it works 
        return handle_error(r, HTTP_STATUS_BAD_REQUEST);

    //Determine the request path 
    r->path = determine_request_path(r->uri); //that function isn't written yet, but it should do it 
    if (stat(r->path, &buf) < 0) { //if it fails 
        return handle_error(r, HTTP_STATUS_INTERNAL_SERVER_ERROR);
    }

    if (S_ISDIR((&buf)->st_mode)) {
        result = handle_browse_request(r);
    } else if (S_ISREG((&buf)->st_mode)) {
        if (access(r->path, X_OK)) {
            result = handle_cgi_request(r);
        } else if (access(r->path, R_OK)) {
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

/*
stat (&buf)
if is_dirbuf->st_mode
    handle broswer
else if is_reg
    if acces s_OK
        cgi
    else if rok
        file request
    else
        handle error with status not found
*/

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
    n = scandir(r->path, &entries, NULL, NULL);

    if (n < 0){
      fprintf(stderr, " Error: unable to scan directory: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }
    if(entries == NULL){
      fprintf(stderr, "Error: failed to open entries: %s\n", strerror(errno));
      return EXIT_FAILURE;
    }

    /* Write HTTP Header with OK Status and text/html Content-Type */
    //maybe -sk
    fprintf(r->file, "HTTP/1.0 200 OK\n");
	  fprintf(r->file, "Content-Type: text/html\n");
    fprintf(r->file, "\r\n");

    /* For each entry in directory, emit HTML list item */
    //use ul

    /* Flush socket, return OK */
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
    if(!(r->file))
        close(r->fd);
    /* Determine mimetype */

    /* Write HTTP Headers with OK status and determined Content-Type */

    /* Read from file and write to socket in chunks */

    /* Close file, flush socket, deallocate mimetype, return OK */
    return HTTP_STATUS_OK;

fail:
    /* Close file, free mimetype, return INTERNAL_SERVER_ERROR */
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
Status  handle_cgi_request(Request *r) {
    FILE *pfs;
    char buffer[BUFSIZ];

    /* Export CGI environment variables from request:
     * http://en.wikipedia.org/wiki/Common_Gateway_Interface */

    /* Export CGI environment variables from request headers */

    /* POpen CGI Script */

    /* Copy data from popen to socket */

    /* Close popen, flush socket, return OK */
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
Status  handle_error(Request *r, Status status) {
    const char *status_string = http_status_string(status);

    /* Write HTTP Header */

    /* Write HTML Description of Error*/

    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
