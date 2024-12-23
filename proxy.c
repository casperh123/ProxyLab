/**
 * TODO: fill in your name and ITU alias
 * @author Casper Holten <casho@itu.dk>
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/* The source code for the proxy is split across three files (including this one). */
#include "proxy.h" // proxy
#include "error.h" // error reporting for ^
#include "http.h"  // http-related things for ^
#include "io.h"    // io-related things for ^
#include "cache.h"
#include "hash.h"
#include <pthread.h>

int main(int argc, char **argv) {
    int listen_fd; // fd for connection requests from clients.

    cache cache = {
        .head = NULL,
        .tail = NULL,
        .size = 0,
        .max_size = MAX_CACHE_SIZE,
        .max_object_size = MAX_OBJECT_SIZE,
        .buffer_size = MAX_LINE
    };

    /* Check command line args for presence of a port number. */
    if (error_args_fatal(argc, argv)) { exit(1); }

    /* Create a `socket`, `bind` it to listen address, configure it to `listen` (for connection requests). */
    listen_fd = create_listen_fd(atoi(argv[1]));

    /* Handle connection requests. */
    while (1) {
        handle_connection_request(listen_fd, &cache);
    }

    return 0; // Indicates "no error" (although this is never reached).
}

void handle_connection_request(int listen_fd, cache *cache) {
    int client_fd; // fd for clients that connect.

    printf("\e[1mawaiting connection request...\e[0m\n");

    /* "Kernel, give me the fd of a connected socket for the next connection request."
       NOTE: this blocks the proxy until a connection arrives.
       https://man7.org/linux/man-pages/man2/accept.2.html (a system call) */
    client_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL); // accept awaiting request
    if (error_accept_fatal(client_fd)) { exit(1); }
    if (error_accept(client_fd)) { return; }

    request_scope *scope_ptr = malloc(sizeof(struct request_scope));

    scope_ptr->cache = cache;
    scope_ptr->client_fd = client_fd;

    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tid, &attr, thread_handle_request, scope_ptr);
}

void thread_handle_request(void *arg) {
    printf("Begin processing request - Spawning new thread\n");

    request_scope *scope = (request_scope *) arg;
    int return_cd;

    handle_request(scope);

    return_cd = close(scope->client_fd);

    if (error_close(return_cd)) {
        /* ignore */
    }
}

void handle_request(struct request_scope *scope) {
    printf("SUCCESS WITH THREAD, REACHED REAL HANDLE REQUEST\n");

    int server_fd; // server file descriptor

    /* String variables */
    char buf[MAX_LINE];
    char method[MAX_LINE];
    char uri[MAX_LINE];
    char version[MAX_LINE];
    char hostname[MAX_LINE];
    char path[MAX_LINE];
    char port[MAX_LINE];
    char request_hdr[MAX_LINE];

    int client_fd = scope->client_fd;
    int return_cd;
    ssize_t num_bytes;

    /* read HTTP Request-line */
    num_bytes = read_line(client_fd, buf);
    if (error_read(num_bytes)) { return; }

    /* print what we just read (it's not null-terminated) */
    printf("%.*s", (int) num_bytes, buf); // typeast is safe; num_bytes <= MAX_LINE
    sscanf(buf, "%s %s %s", method, uri, version);

    /* Ignore non-GET requests (your proxy is only tested on GET requests). */
    if (error_non_get(method)) { return; }

    int key = hash(uri);
    cache_node* cache_hit = cache_get(scope->cache, key);

    if(cache_hit != NULL) {
        printf("\033[33mCACHE HIT:\033[0m %s\n", uri);

        num_bytes = write_all(client_fd, cache_hit->data, cache_hit->size);

        printf("\033[33mCACHE HIT:\033[0m Wrote some bytes to: %s\n", uri);

        if (error_write_client(client_fd, num_bytes)) {
            return;
        }
    } else {
        printf("\033[33mCACHE MISS:\033[0m %s\n", uri);

        /* Parse URI from GET request */
        parse_uri(uri, hostname, path, port);


        /* Set the request header */
        return_cd = set_request_header(request_hdr, hostname, path, port, client_fd);
        if (error_header(return_cd)) { return; }

        /* Create the server fd. */
        server_fd = create_server_fd(hostname, port);
        if (error_socket_server(server_fd)) { return; }

        /* Write the request (header) to the server. */
        return_cd = write_all(server_fd, request_hdr, strlen(request_hdr));
        if (error_write_server(server_fd, return_cd)) { return; }

        char* response_buffer = malloc(MAX_OBJECT_SIZE);
        size_t response_size = 0;

        do {
            num_bytes = read(server_fd, buf, MAX_LINE);
            if (error_read_server(server_fd, num_bytes)) {
                free(response_buffer);
                return;
            }

            //Always write to client first
            num_bytes = write_all(client_fd, buf, num_bytes);
            if (error_write_client(client_fd, num_bytes)) {
                free(response_buffer);
                return;
            }

            //Only cache if we haven't exceeded size
            if(response_size + num_bytes <= MAX_OBJECT_SIZE) {
                memcpy(response_buffer + response_size, buf, num_bytes);
                response_size += num_bytes;
            }
        } while (num_bytes > 0);

        //Only cache if we got everything
        if (response_size <= MAX_OBJECT_SIZE) {
            cache_put(scope->cache, key, response_buffer, response_size);
        }

        free(response_buffer);
    }

    /* success; close the file descrpitor. */
    return_cd = close(server_fd);
    if (error_close_server(return_cd)) {
        /* ignore */
    }
}

int create_listen_fd(int port) {
    /* File descriptors */
    int listen_fd; // fd for connection requests from clients.

    /* Return code */
    int return_cd;

    /* Socket address (on which proxy shall listen for connection requests) (populated soon).
       https://man7.org/linux/man-pages/man3/sockaddr.3type.html */
    struct sockaddr_in listen_addr;

    printf("\e[1mcreating listen_fd\e[0m\n");

    /* Set socket address (on which proxy shall listen for connection requests). */
    set_listen_socket_address(&listen_addr, port);

    /* "Kernel, make me a socket." (for listening to client connection requests).
       https://man7.org/linux/man-pages/man2/socket.2.html (a system call) */
    listen_fd = socket(listen_addr.sin_family, SOCK_STREAM, 0);
    if (error_socket_fatal(listen_fd)) { exit(1); }

    /* "Kernel, if you think the address I'm binding to is already in use, then
       this socket may reuse the address." (optional)
       NOTE: quality-of-life; it takes kernel ~1 min to free up an address; w/o
       this, after proxy stopped, you have to wait a bit before you can start again).
       https://man7.org/linux/man-pages/man2/setsockopt.2.html (a system call) */
    return_cd = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    if (error_socket_option(return_cd)) {
        /* ignore */
    }

    /* "Kernel, bind it to this socket address" (i.e. where proxy shall listen).
       https://man7.org/linux/man-pages/man2/bind.2.html (a system call) */
    return_cd = bind(listen_fd, (struct sockaddr *) &listen_addr, sizeof(listen_addr));
    if (error_bind_fatal(return_cd)) { exit(1); }

    /* "Kernel, oh btw, that socket? Make it passive." (it's for connection requests)
       https://man7.org/linux/man-pages/man2/listen.2.html (a system call) */
    return_cd = listen(listen_fd, LISTENQ);
    if (error_listen_fatal(return_cd)) { exit(1); }

    printf("\e[1mlisten_fd ready\e[0m\n");

    return listen_fd;
}

/* set the proxy socket address (where it listens for connection requests). */
void set_listen_socket_address(struct sockaddr_in *listen_addr, int port) {
    memset(listen_addr, '0', sizeof(struct sockaddr_in)); // zero out the address
    listen_addr->sin_family = AF_INET;
    listen_addr->sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr->sin_port = htons(port);
    /* NOTE: we /should/ use `getnameinfo` & `getaddrinfo` (in real world, so should you).
       with `getaddrinfo`, we get a list of potential socket addresses, and for each
       socket address in the list, we should attempt to create + bind a socket to it
       (stopping on the first successful `socket` (i.e. create)  and 'bind'). why:
        * more robust (can bind to 32-bit and 256-bit addresses, whichever server has)
        * more secure (an attacker on `cos` cannot hijack this socket by
                       binding to a more specific address than INADDR_ANY).
       instead, here we hard-code port, pick 32-bit IP addresses, and all available interfaces.
       why: because I know cos supports this, and it is simpler; `getaddrinfo` is
       intimidating for the uninitiated. (why: check out the server socket code.) */
    printf("\033[32msuccess:\033[0m set socket address of proxy.\n");
}

int create_server_fd(char *hostname, char *port) {
    int server_fd;
    int return_cd;

    struct addrinfo *cand_ai; // pointer to heap-allocated candidate server addresses (free this!)

    /* Get list of candidate server socket addresses. */
    return_cd = get_server_socket_address_candidates(&cand_ai, hostname, port);
    if (error_address_server(return_cd)) { return -1; }

    struct addrinfo *curr_ai; // pointer to current candidate server address in the above list.

    /* produces a socket (server_fd) bound to the first candidate address (in cand_ai)
       for which creating (resp. binding) a socket for (resp. to) it was successful. */
    for (curr_ai = cand_ai; curr_ai != NULL; curr_ai = curr_ai->ai_next) {
        /* "Kernel, make me a socket." (for curr_ai)
           https://man7.org/linux/man-pages/man2/socket.2.html (a system call) */
        server_fd = socket(curr_ai->ai_family, curr_ai->ai_socktype, curr_ai->ai_protocol);
        if (server_fd == -1)
            continue; // try the next ai.

        /* "Kernel, please (attempt to) connect to said socket."
           https://man7.org/linux/man-pages/man2/connect.2.html (a system call) */
        return_cd = connect(server_fd, curr_ai->ai_addr, curr_ai->ai_addrlen);
        //return_cd = connect ( server_fd, (struct sockaddr *)&curr_ai, sizeof(curr_ai) );
        if (return_cd < 0) { printf("failure connecting to socket. trying next one.\n"); }
        if (return_cd == 0)
            break; // success

        /* couldn't bind the socket to curr_ai. try the next ai. */
        close(server_fd);
    }
    /* free up the heap-allocated linked list. */
    freeaddrinfo(curr_ai);

    /* report errors if any. */
    if (return_cd < 0) { return -1; }

    /* success; return the server fd. */
    return server_fd;
}

int get_server_socket_address_candidates(struct addrinfo **cand_ai, char *hostname, char *port) {
    struct addrinfo hints_ai; // hints for proposing candidate server addresses (i.e. for generating cand_ai)
    /* set hints. network socket, numeric port, avoid IPv6 socket for hosts that don't support those. */
    memset(&hints_ai, 0, sizeof(struct addrinfo));
    hints_ai.ai_socktype = SOCK_STREAM;
    hints_ai.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG;
    return getaddrinfo(hostname, port, &hints_ai, cand_ai);
}
