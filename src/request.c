/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

int parse_request_method(Request *r);
int parse_request_headers(Request *r);

/**
 * Accept request from server socket.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Newly allocated Request structure.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
Request * accept_request(int sfd) {

    Request * r = calloc (1, sizeof(Request));  // Allocate request struct
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    /* Initialize the headers list */
    r->headers = calloc(1, sizeof(Header));

    /* Accept a client */
    r->fd = accept(sfd, &raddr, &rlen);
    if(r->fd <0){
        fprintf(stderr, "Unable to accept: %s\n", strerror(errno));
        goto fail;
    }

    /* Lookup client information */
    if(getnameinfo(&raddr, rlen, r->host, sizeof(r->host), r->port, sizeof(r->port), NI_NUMERICHOST | NI_NUMERICSERV) != 0){
        fprintf(stderr, "Unable to get client info: %s\n", strerror(errno));
        goto fail;
    }

    /* Open socket stream */
    r->file = fdopen(r->fd, "w+");
    if(!r->file){
        fprintf(stderr, "Unable to fdopen: %s\n", strerror(errno));
        close(r->fd);
    }
    log("Accepted request from %s:%s", r->host, r->port);
    return r;

fail:
    /* Deallocate request struct */
    free_request(r);
    return NULL;
}

/**
 * Deallocate request struct.
 *
 * @param   r           Request structure.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void free_request(Request *r) {

    if (!r) return;

    /* Close socket or fd */
    close(r->fd);

    /* Free allocated strings */
    free(r->method);
    free (r->uri);
    free(r->path);
    free(r->query);

    /* Free headers */
    struct header * curr = r->headers->next;
    struct header * next;
    while (curr) {
      next = curr->next;
      free(curr->name);
      free(curr->value);
      free(curr);
      curr = next;
    }
    free(r->headers);

    /* Free request */
    free(r);
}


/**
 * Parse HTTP Request.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int parse_request(Request * r) {

    /* Parse HTTP Request Method */
    if (parse_request_method(r) < 0) return -1;
    //char buffer[BUFSIZ];

    //if(!(fgets(buffer, BUFSIZ, r->file)))  return -1;

    /* Parse HTTP Request Headers*/
    if(parse_request_headers(r) < 0) return -1;

    return 0;
}

/**
 * Parse HTTP Request Method and URI.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int parse_request_method(Request *r) {
    char buffer[BUFSIZ];
    char *uri; //used for splitting up the uri into uri and query
    char *method;

    /* Read line from socket */

    if(!fgets(buffer, BUFSIZ, r->file)) goto fail;
    fprintf(stderr, "REQUEST.c: Here's the buffer: \"%s\"\n", buffer);

    /* Parse method and uri */
    if(!(method = strtok(buffer, WHITESPACE))) goto fail;
    r->method = strdup(method); //need to strdup to allocate mem
    uri = strtok(NULL, " \t\n");
    if (!uri) goto fail;

    /* Parse query from uri */

    char * end_uri = strchr(uri, '?');
    if (end_uri != NULL) {          // Taking query out of URI
      r->query = strdup(end_uri + 1);
      if(!r->query) goto fail;
    } else {
      r->query = strdup("");
    }
    //end_uri = NULL;
    if (!(uri = strtok(uri, "? \t\n"))) goto fail;
		if (strcmp(r->uri, "/") == 0) {
			r->uri = strdup("/html/index.html");
		} else {
    	r->uri = strdup(uri); //allocating memory for the correct uri
		}
    if(!r->uri) goto fail;

    /* Record method, uri, and query in request struct */
    debug("HTTP METHOD: %s", r->method);
    debug("HTTP URI:    %s", r->uri);
    debug("HTTP QUERY:  %s", r->query);

    return 0;

fail:
    return -1;
}


/**
 * Parse HTTP Request Headers.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int parse_request_headers(Request * r) {

    char buffer[BUFSIZ];
    char * name;
    //char * value;

    /* Parse headers from socket */
    while (fgets(buffer, BUFSIZ, r->file) && strlen(buffer) > 2) {

      /* Create header */
      struct header * curr = NULL;
      if ( !(curr = malloc(sizeof(struct header))) ) {
          goto fail;
      }

      /* Set the next variable */
      curr->next = r->headers->next;

      /* Look for ':' */
      char * c;
      if (!(c = strchr(buffer, ':'))) {
        free(curr);
        goto fail;
      }

      /* Set the header value */
      if (!(curr->value = strdup(c + 2))) {
          free(curr);
          goto fail;
      }

      /* Set header name */
      if (!(name = strtok(buffer, ":"))) {
          free(curr->value);
          free(curr);
          goto fail;
      }

      /* Set header name */
      if (!(curr->name = strdup(name))) {
          free(curr->value);
          free(curr);
          goto fail;
      }

      /* Add header to linked list */
      r->headers->next = curr;

    }

#ifndef NDEBUG
    for (struct header * header = r->headers; header; header = header->next) {
    	debug("HTTP HEADER %s = %s", header->name, header->value);
    }
#endif
    return 0;

fail:
    return -1;

}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
