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

typedef struct connection {
    int conn_fd;
    pthread_t thread;
    int idx;
} connection;

// ring buffer, queue    or    malloc, free?????
// connection connections[MAX_CONNECTIONS] = {-1};
int connections[MAX_CONNECTIONS] = {-1};

void thread_cleanup(connection * connection) {
    // done elsewhere for now
    // int ret = close(connection->conn_fd);
    // pthread_cleanup_pop(connection);
    // do stuff
}

void * dummy() {
    sleep(1);
}

void server_listen(char * address, uint16_t port, void * handler_func) {
    // struct rlimit old = {};
    // getrlimit(RLIMIT_NPROC, &old);
    // printf("old: %ld, %ld\n", old.rlim_cur, old.rlim_max);
    // struct rlimit new = {
    //     .rlim_cur = 10,
    //     .rlim_max = 10,
    // };
    // setrlimit(RLIMIT_NPROC, &new);
    // getrlimit(RLIMIT_NPROC, &old);
    // printf("new: %ld, %ld\n", old.rlim_cur, old.rlim_max);

    // NOTE: as currently implemented, this will potentially use up all "availble" threads.. any otherwise long-running threads required by the application should be created prior to this function...
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
        // TODO: SET KEEPALIVE, TIMEOUTS AND/OR NONBLOCK???
        uint8_t keep_alive = 1;
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, 1);

        // struct timeval timeout = {
        //     .tv_sec = 0,
        //     .tv_usec = 500 * 1000, // 500ms
        // };
        // setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
        // setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

        if (bind(fd, (struct sockaddr *)(&address), sizeof(struct sockaddr_in)) == -1) {
            // check errno for error
            printf("error binding, %d\n", errno);
        } else {
            if (listen(fd, 1024) == -1) {
                printf("Error listening\n");
            } else {
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, 1);

                int idx = 0;
                // int conn = accept(fd, NULL, NULL);
                // TODO: use this for now.... until I identify best option
                int conn = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                while (conn != -1) {
                    connections[idx] = conn;
                    pthread_t thread;
                    int ret = pthread_create(&thread, &attr, handler_func, &connections[idx]);
                    while (ret != 0) {
                        usleep(1000); // sleep for 1 ms
                        ret = pthread_create(&thread, &attr, handler_func, &connections[idx]);
                    }
                    // TODO: 
                    // assume, connnection handlers are done with &conn_fd by this point...
                    // Should I block until flag is set by caller? or something of that sort?
                    if (++idx == MAX_CONNECTIONS) {
                        idx = 0;
                    }
                    conn = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
                    // conn = accept(fd, NULL, NULL);
                }
                if (conn == -1) {
                    printf("ERROR ACCEPTING connections\n");
                }
            }
        }
    }
}