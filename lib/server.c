#include "server.h"
#include "logger.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <fcntl.h>

#include "server_connection_timer.h"

#define MAX_CONNECTIONS (1 << 16)

// TODO: think about default... LMAO...
#define INITIAL_CONNECTION_TIMEOUT_S 15

typedef struct thread_arg {
    connection_handler_t * handler;
    int fd;
} thread_arg;

static void * handler_wrapper_func(void * arg);
static void process_sockopts(int fd);

extern void serverSetConnectionTimeout(int fd, uint32_t timeout_s) {
    if (timeout_s > INITIAL_CONNECTION_TIMEOUT_S) {
        timerSetTimeout(fd, timeout_s);
    }
}

extern void serverListen(const char * address, const uint16_t port, connection_handler_t handler) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        // check errno for error
        printf("error creating socket\n");
    } else {
        struct in_addr addr; 
        if (inet_aton(address, &addr) == 0) {
            printf("Error parsing address string, %s\n", address);
        } else {
            struct sockaddr_in a = {
                AF_INET,
                ntohs(port),
                addr,
            };
            if (bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in)) == -1) {
                // check errno for error
                printf("error binding, %d\n", errno);
                return;
            } else {
                // queue up to MAX_CONNECTIONS before refusing connections.
                if (listen(fd, MAX_CONNECTIONS) == -1) {
                    printf("Error listening\n");
                } else {
                    printf("Listening on %s:%u\n", address, port);
                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setdetachstate(&attr, 1);
     
                    int conn = accept(fd, NULL, NULL);
                    while (conn != -1) {
                        // TODO: might reach some stack limit...
                        pthread_t thread;
                        thread_arg * arg = (thread_arg *)malloc(sizeof(thread_arg));
                        arg->fd = conn;
                        arg->handler = handler;
                        // wrapper func is responsible for freeing... 
                        int ret = pthread_create(&thread, &attr, handler_wrapper_func, arg);
                        while (ret != 0) {
                            usleep(1000); // sleep for 1 ms
                            // TODO: is it safe to assume this will eventually return 0?
                            ret = pthread_create(&thread, &attr, handler_wrapper_func, arg);
                        }
                        conn = accept(fd, NULL, NULL);
                    }
                    if (conn == -1) {
                        printf("ERROR ACCEPTING connections, errno %d\n", errno);
                    }
                }
            }
        }
    }
}

static void * handler_wrapper_func(void * arg) {
    thread_arg * a = (thread_arg *)arg;
    connection_handler_t * handler = a->handler;
    int fd = a->fd;
    free(a); // free before function call, so that it can terminate thread however it wants... 

    // TODO: SIGHANDLING, seccomp?
    //  SIGSEGV 
    //  SIGFPE?
    //  SIGSYS?
    //  SIGTRAP?
    //  SIGXCPU?
    //  SIGXFSZ?

    // it's lame that I need to do this for each 'connection'. And 'socket' is same as 'connection' which is the same as 'socket'? lol
    // I thought it was a 1 (socket) to many (connections) kind of thing (at least for bind).

    // what I meant is that
    // socket == source:port,destination:port == connection
    //  not, socket == destination:port and source:port,destination:port == connection
    //  in other words, the api doesn't distinguish between the two. I thought socket was a higher level abstraction of the interface, vs an instance of the interface?
    //  idk...
    // in hindsight, duh lol...
    process_sockopts(fd);
    timerAddConnection(fd, INITIAL_CONNECTION_TIMEOUT_S);
    handler(fd);
    return NULL;
}

// TODO: this is not what I thought? hmm... need to implement connection timer... though, it's really only a nice to have...

//  as far as terminating an active connection/thread...
//  setting SO_RCVTIMEO and SO_SNDTIMEO to a very small usec value, 
//      should cause read errors and a properly implemented application *should* gracefully terminate thread...
static void server_set_connection_sockopts(int fd) {
    socklen_t byte_len = sizeof(uint8_t);
    socklen_t uint32_t_len = sizeof(uint32_t);
    socklen_t timeval_len = sizeof(struct timeval);

    // TODO: set RECVBUFSIZE and SENDBUFSIZE, arg it up?
    uint8_t keep_alive = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, byte_len);
    struct timeval timeout = {
        .tv_sec = INITIAL_CONNECTION_TIMEOUT_S,
        .tv_usec = 0,
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);

    // read and print relevant options, ignore any error's reading... this is a nice to have...
    if (LOGGER_LEVEL >= LOGGER_DEBUG) {
        uint32_t rcv_buf_size = 0;
        getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcv_buf_size, &uint32_t_len); // why pointer to len? also lame
        uint32_t snd_buf_size = 0;
        getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, &uint32_t_len);
        struct timeval rcv_timeout = {0};
        getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
        struct timeval snd_timeout = {0};
        getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, &timeval_len);
        uint8_t ret_keep_alive = 1;
        getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &ret_keep_alive, &byte_len);

        loggerPrintf(LOGGER_DEBUG, "SOCK OPTS: KEEP_ALIVE %u, SO_RCVBUF %u, SO_RCVTIMEO %lds %ldus, SO_SNDBUF %u, SO_SNDTIMEO %lds %ldus\n", ret_keep_alive, rcv_buf_size, rcv_timeout.tv_sec, rcv_timeout.tv_usec, snd_buf_size, snd_timeout.tv_sec, snd_timeout.tv_usec);
    }
}