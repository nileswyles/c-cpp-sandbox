#include "server.h"

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CONNECTIONS (1 << 16)

typedef struct connection {
    int conn_fd;
    pthread_t thread;
    int idx;
} connection;

// ring buffer, queue    or    malloc, free?????
connection connections[MAX_CONNECTIONS] = {-1};

void thread_cleanup(connection * connection) {
    // done elsewhere for now
    // int ret = close(connection->conn_fd);
    // pthread_cleanup_pop(connection);
    // do stuff
}

void server_listen(char * address, uint16_t port, void * handler_func) {
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

        if (bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr_in)) == -1) {
            // check errno for error
            printf("error binding, %d\n", errno);
        } else {
            if (listen(fd, 1024) == -1) {
                printf("Error listening\n");
            } else {
                int conn = accept4(fd, NULL, NULL, SOCK_NONBLOCK|SOCK_CLOEXEC);
                // while (conn != -1) {
                if (conn != -1) {
                    connections[idx].conn_fd = conn;
                    connections[idx].idx = idx;
                    int ret = pthread_create(&connections[idx].thread, NULL, handler_func, &connections[idx].conn_fd);
                    // check ret...
                    // pthread_cleanup_push(thread_cleanup, &connections[idx]);
                    pthread_join(connections[idx].thread, NULL);
                    idx++;
                }
                if (conn == -1) {
                    printf("ERROR ACCEPTING connections\n");
                }
            }
        }
    }
}