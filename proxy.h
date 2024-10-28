/* Macro constants */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define LISTENQ 1024

#ifndef MAX_LINE
#define MAX_LINE 8192 // HTTP Semantics (RFC 9110) recommends >= 8000 characters.
#endif/*MAX_LINE*/
#include "cache.h"

typedef struct request_scope {
    cache *cache;
    int client_fd;
} request_scope;

void handle_request(request_scope *scope);
void thread_handle_request(void *arg);
int  create_listen_fd ( int port);
void handle_connection_request ( int listen_fd, cache *cache );
void get_client_socket_address ( struct sockaddr *client_addr, char *hostname, char *port);
void set_listen_socket_address ( struct sockaddr_in *listen_addr, int port );
int  get_server_socket_address_candidates ( struct addrinfo **cand_ai, char* hostname, char* port );
int  create_server_fd ( char* hostname, char* port );
