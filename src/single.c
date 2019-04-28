/* single.c: Single User HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

/**
 * Handle one HTTP request at a time.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Exit status of server (EXIT_SUCCESS).
 **/
int single_server(int sfd) {

    Status status;
    /* Accept and handle HTTP request */
    while (true) {
    	/* Accept request */
        Request * request = accept_request(sfd);
        if (!request) return HTTP_STATUS_BAD_REQUEST;
	/* Handle request */
        status = handle_request(request);
	/* Free request */
        free_request(request);
    }

    /* Close server socket */
    close(sfd);
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
