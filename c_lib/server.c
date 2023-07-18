#include "server.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/resource.h>

#define MAX_CONNECTIONS (1 << 16)

typedef struct thread_arg {
    connection_handler_t * handler;
    int fd;
} thread_arg;

static void * handler_wrapper_func(void * handler) {
    thread_arg * h = (thread_arg *)handler;
    (*h).handler((*h).fd); // Note: this should not call pthread_exit, else cleanup is skipped.. meaning memory leak...
    free(h);
}

void server_listen(char * address, uint16_t port, connection_handler_t handler) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int idx = 0;
    if (fd == -1) {
        // check errno for error
        printf("error creating socket\n");
    } else {
        // REMEMBER, Big-Endianness is prominent in network protocols like IP... (network order) - send most significant byte first...
        // so, call ntohs to convert port... addr is already defined in network order.
        static const uint8_t addr[] = {0, 0, 0, 0}; // addr uint32
        struct sockaddr_in address = {
            AF_INET,
            ntohs(port),
            *(in_addr_t*)addr,
        };
        uint8_t keep_alive = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, 1);

        struct timeval timeout = {
            .tv_sec = 15,
            .tv_usec = 0, // 15s
        };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

        if (bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr_in)) == -1) {
            // check errno for error
            printf("error binding, %d\n", errno);
        } else {
            // queue up to MAX_CONNECTIONS before refusing connections.
            if (listen(fd, MAX_CONNECTIONS) == -1) {
                printf("Error listening\n");
            } else {
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, 1);

                int conn = accept(fd, NULL, NULL);
                while (conn != -1) {
                    pthread_t thread;
                    thread_arg * arg = malloc(sizeof(thread_arg));
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