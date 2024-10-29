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

static uint32_t INITIAL_CONNECTION_TIMEOUT_S = 15;
static uint32_t INITIAL_SOCKET_TIMEOUT_S = 2;

typedef struct thread_arg {
    connection_handler_t * handler;
    int fd;
} thread_arg;

static void * handler_wrapper_func(void * arg);
static void process_sockopts(int fd);

extern void serverDisableConnectionTimeout(int fd) {
    timerRemoveConnection(fd, false);
}

extern void serverSetConnectionTimeout(int fd, uint32_t timeout_s) {
    if (timeout_s > INITIAL_CONNECTION_TIMEOUT_S) {
        timerSetTimeout(fd, timeout_s);
    }
}

extern void serverSetInitialConnectionTimeout(int fd, uint32_t timeout_s) {
    serverSetConnectionTimeout(fd, timeout_s);
    INITIAL_CONNECTION_TIMEOUT_S = timeout_s;
}

extern void serverSetSocketTimeout(int fd, uint32_t timeout_s) {
    socklen_t timeval_len = sizeof(struct timeval);

    struct timeval timeout = {
        .tv_sec = timeout_s,
        .tv_usec = 0,
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);
}

extern void serverSetInitialSocketTimeout(int fd, uint32_t timeout_s) {
    serverSetSocketTimeout(fd, timeout_s);
    INITIAL_SOCKET_TIMEOUT_S = timeout_s;
}

extern uint32_t serverGetConnectionTimeout(int fd) {
    return timerGetTimeout(fd);
}

extern uint32_t serverGetSocketTimeout(int fd) {
    socklen_t timeval_len = sizeof(struct timeval);
    struct timeval rcv_timeout = {0};
    getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
    return rcv_timeout.tv_sec;
}

extern void serverListen(const char * address, const uint16_t port, connection_handler_t handler) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        // check errno for error
        loggerPrintf(LOGGER_DEBUG, "Error creating socket.\n");
    } else {
        struct in_addr addr; 
        if (inet_aton(address, &addr) == 0) {
            loggerPrintf(LOGGER_DEBUG, "Error parsing address string, %s\n", address);
        } else {
            struct sockaddr_in a = {
                AF_INET,
                ntohs(port),
                addr,
            };
            if (bind(fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in)) == -1) {
                // check errno for error
                loggerPrintf(LOGGER_DEBUG, "error binding, %d\n", errno);
                return;
            } else {
                // queue up to MAX_CONNECTIONS before refusing connections.
                if (listen(fd, MAX_CONNECTIONS) == -1) {
                    loggerPrintf(LOGGER_DEBUG, "Error listening\n");
                } else {
                    loggerPrintf(LOGGER_DEBUG, "Listening on %s:%u\n", address, port);
                    timerStart();
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

                        // TODO: performance profiling...
                        //  Can multiplex/thread only for websocket upgrades? This means using poll on these file descriptors to start parsing.
                        //  but again, this complicates the api handler/controller code...
                        int ret = pthread_create(&thread, &attr, handler_wrapper_func, arg);
                        while (ret != 0) {
                            usleep(1000); // sleep for 1 ms
                            ret = pthread_create(&thread, &attr, handler_wrapper_func, arg);
                        }
                        loggerPrintf(LOGGER_DEBUG, "Failed to create thread! Trying again.\n");
                        conn = accept(fd, NULL, NULL);
                    }
                    timerStop();
                    if (conn == -1) {
                        loggerPrintf(LOGGER_DEBUG, "ERROR ACCEPTING connections, errno %d\n", errno);
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
    process_sockopts(fd);
    timerAddConnection(fd, INITIAL_CONNECTION_TIMEOUT_S);
    handler(fd);
    timerRemoveConnection(fd, true);
    return NULL;
}

#define RCV_SND_BUF_DEFAULTS 131072

// TODO: review openssl and socket documentation as well as throughly test this code because uncertainty is a thing.
static void process_sockopts(int fd) {
    socklen_t byte_len = sizeof(uint8_t);
    socklen_t uint32_t_len = sizeof(uint32_t);
    socklen_t timeval_len = sizeof(struct timeval);

    uint32_t buf_size = RCV_SND_BUF_DEFAULTS;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buf_size, uint32_t_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buf_size, uint32_t_len);

    uint8_t keep_alive = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, byte_len);

    // close connection immediately
    uint8_t linger = 0;
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &linger, byte_len);

    // ! IMPORTANT
    // For reads,
    // should be minimum value greater than max(SO_RCVBUF, READER_BUF_SIZE)/128kbps (aribitrary minimum connection speed)
    //  default RCVBUF == 131072, READER_BUF_SIZE == 8096, so ~1.024 seconds. Let's round to 2 seconds.
    struct timeval timeout = {
        .tv_sec = INITIAL_SOCKET_TIMEOUT_S,
        .tv_usec = 0,
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);

    // read and print relevant options, ignore any error's reading... this is a nice to have...
    loggerExec(LOGGER_DEBUG_VERBOSE, 
        uint32_t rcv_buf_size = 0;
        getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcv_buf_size, &uint32_t_len); // why pointer to len? also lame
        uint32_t snd_buf_size = 0;
        getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_buf_size, &uint32_t_len);
        uint32_t rcv_lo_wat = 0;
        getsockopt(fd, SOL_SOCKET, SO_RCVLOWAT, &rcv_lo_wat, &uint32_t_len);
        uint32_t snd_lo_wat = 0;
        getsockopt(fd, SOL_SOCKET, SO_SNDLOWAT, &snd_lo_wat, &uint32_t_len);
        struct timeval rcv_timeout = {0};
        getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
        struct timeval snd_timeout = {0};
        getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &snd_timeout, &timeval_len);
        uint8_t ret_keep_alive = 1;
        getsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &ret_keep_alive, &byte_len);

        loggerPrintf(LOGGER_DEBUG_VERBOSE, "SOCK OPTS: \n KEEP_ALIVE %u\n SO_RCVBUF %u\n SO_RCVLOWAT %u\n SO_SNDLOWAT %u\n SO_RCVTIMEO %lds %ldus\n SO_SNDBUF %u\n SO_SNDTIMEO %lds %ldus\n", 
            ret_keep_alive, rcv_buf_size, 
            rcv_lo_wat, snd_lo_wat,
            rcv_timeout.tv_sec, rcv_timeout.tv_usec, 
            snd_buf_size, snd_timeout.tv_sec, snd_timeout.tv_usec);
    );
}