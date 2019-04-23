/* socket.c: Simple Socket Functions */

#include "spidey.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Allocate socket, bind it, and listen to specified port.
 *
 * @param   port        Port number to bind to and listen on.
 * @return  Allocated server socket file descriptor.
 **/

/* ~QUESTIONS~
 * ASK BUI:
 * This file as it stands gets the address info of the HOST&PORT and loops ...
 * ... until a socket can be binded to and listened to. It seems like we ...
 * ... should be getting the address info in another file and performing the ...
 * ... loop in that same file in which we call this function. Then if we ...
 * ... return -1 or something then it obviously failed and we keep ...
 * ... looping in the other file until a valid file descriptor is returned. ...
 * ... Is this assumption correct? And where is the overall FOR Loop? I ...
 * ... still need to look @ all the files thoroughly but I wanted to get ...
 * ... some advice from the person who formatted all the files.
 *
 * Starting at spidey.c, how do things get distributed downwards to the ...
 * ... tree of files? Cause in main(), it says to parse the options and then ...
 * ... immediately listen to the socket (but we never got the addrinfo).
 *
 * If it's all this file, ask if HOST should just be "localhost"
 *
*/

int socket_listen(const char * port) {
  /* Lookup server address information */
  struct addrinfo * results;
  struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_TYPE,
    .ai_flags = AI_PASSIVE
  };

  int status = getaddrinfo(HOST, PORT, &hints, &results)
  if(status != 0) return -1;

  /* For each server entry, allocate socket and try to connect */
  int socket_fd = -1;

  for (struct addrinfo * p = results; p != NULL && socket_fd < 0; p = p->ai_next) {
    /* Allocate socket */
    server_fd = socket(p->ai_family, p->ai_socktype, p->ai_prototype)
    if (server_fd < 0) continue;
    /* Bind socket */
    if ( bind(server_fd, p->ai_addr, p->ai_addrlen) < 0) {
      fprintf(stderr, "Unable to bind: %s\n", strerror(errno));
      close(server_fd);
      server_fd = -1;
      continue;
    }
    /* Listen to socket */
    if (listen(server_fd, SOMAXCONN) < 0) {
      fprintf(stderr, "Unable to listen to socket: %s\n", strerror(errno));
      close(server_fd);
      server_fd = -1;
    }
  }
  /* release allocated address info */
  //not sure if we need to check here
  if (server_fd < 0) {
    fprintf(stderr, "Server does not exist: %s\n", sterror(errno));
    return EXIT_FAILURE;
  }
  freeaddrinfo(results);
  return socket_fd;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
