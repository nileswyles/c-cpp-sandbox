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

#include "threads/etasker.h"

#define MAX_CONNECTIONS (1 << 16)

void Server::disableConnectionTimeout(int fd) {
    tasker.setThreadTimeout(SIZE_MAX);
}

void Server::setConnectionTimeout(int fd, uint64_t timeout_s) {
    if (timeout_s > INITIAL_CONNECTION_TIMEOUT_S) {
        tasker.setThreadTimeout(timeout_s);
    }
}

void Server::setInitialConnectionTimeout(int fd, uint64_t timeout_s) {
    tasker.setThreadTimeout(timeout_s);
    tasker.initial_timeout_s = timeout_s;
}

void Server::setSocketTimeout(int fd, uint64_t timeout_s) {
    socklen_t timeval_len = sizeof(struct timeval);

    struct timeval timeout = {
        .tv_sec = static_cast<__time_t>(timeout_s),
        .tv_usec = 0,
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);
}

void Server::setInitialSocketTimeout(int fd, uint64_t timeout_s) {
    this->setSocketTimeout(fd, timeout_s);
    this->initial_socket_timeout_s = timeout_s;
}

uint64_t Server::getConnectionTimeout(int fd) {
    return tasker.getThreadTimeout();
}

uint64_t Server::getSocketTimeout(int fd) {
    socklen_t timeval_len = sizeof(struct timeval);
    struct timeval rcv_timeout = {0};
    getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
    return rcv_timeout.tv_sec;
}

void Server::listen(const char * address, const uint16_t port) {
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
                if (::listen(fd, MAX_CONNECTIONS) == -1) {
                    loggerPrintf(LOGGER_DEBUG, "Error listening\n");
                } else {
                    loggerPrintf(LOGGER_INFO, "Listening on %s:%u\n", address, port);
                    pthread_attr_t attr;
                    pthread_attr_init(&attr);
                    pthread_attr_setdetachstate(&attr, 1);
     
                    int conn = accept(fd, NULL, NULL);
                    while (conn != -1) {
                        while (0 != this->onConnection(conn) && errno == EAGAIN) {
                            // block here until can create new threads again... this is somewhat likely?
                            //  listen is configured to queue up to MAX_CONNECTIONS so backlog shouldn't be an issue.
                            //  don't care about exceptions... just stop the entire process for now.

                            // TODO: can either poll logs for this message or hit some endpoint to automatically scale up? or is it even to be handled here? just pre-calculate?
                            loggerPrintf(LOGGER_INFO, "Unable to create new threads. Too many persistent connections? Time to scale!\n");
                        }
                        conn = accept(fd, NULL, NULL);
                    }
                    if (conn == -1) {
                        loggerPrintf(LOGGER_DEBUG, "ERROR ACCEPTING connections, errno %d\n", errno);
                    }
                }
            }
        }
    }
}