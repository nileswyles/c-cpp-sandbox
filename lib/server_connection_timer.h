#ifndef SERVER_CONNECTION_TIMER_H
#define SERVER_CONNECTION_TIMER_H

#if defined __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef LOGGER_SERVER_CONNECTION_TIMER
#define LOGGER_SERVER_CONNECTION_TIMER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_SERVER_CONNECTION_TIMER
#include "logger.h"

typedef struct Connection {
    int fd;
    uint64_t start_s;
    uint32_t timeout_s;
    Connection * next;
    Connection * prev;
} Connection;

static Connection * start = NULL;
static Connection * end = NULL;

static uint64_t runtime = 0;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t tick_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t tick_thread;
static pthread_t process_thread;

static bool run = false;

static void timerCloseConnection(int fd);
static int timerStart();
static void timerStop();
static void timerProcess(void * arg);
static void timerSetTimeout(int fd, uint32_t timeout_s);
static void timerAddConnection(int fd, uint32_t timeout_s);

static void timerCloseConnection(int fd) {
    socklen_t timeval_len = sizeof(struct timeval);
    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = 1,
    };
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, timeval_len);
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, timeval_len);

    if (LOGGER_LEVEL >= LOGGER_DEBUG) {
        struct timeval rcv_timeout = {0};
        getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_timeout, &timeval_len);
        loggerPrintf(LOGGER_DEBUG, "SO_RCVTIMEO %lds %ldus\n", rcv_timeout.tv_sec, rcv_timeout.tv_usec);
        getsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &rcv_timeout, &timeval_len);
        loggerPrintf(LOGGER_DEBUG, "SO_SNDTIMEO %lds %ldus\n", rcv_timeout.tv_sec, rcv_timeout.tv_usec);
    }
}

static int timerStart() {
    run = true;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    return pthread_create(&process_thread, &attr, timerProcess, NULL);
}

static void timerStop() {
    run = false;
}

static void timerProcess(void * arg) {
    while(run) {
        pthread_mutex_lock(&mutex);
        Connection * current = start;
        while (current != NULL) {
            Connection * prev = current->prev;
            Connection * next = current->next;
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            if (current->start_s + current->timeout_s >= ts.tv_sec) {
                timerCloseConnection(current->fd());

                // remove item from linked list.
                if (prev == NULL) { // if first item in list...
                    start = next; 
                } else {
                    prev->next = next;
                }
                if (next == NULL) { // if last item in list...
                    end = prev;
                } else {
                    next->prev = prev;
                }
                free(current);
            }
            current = next;
        }
        pthread_mutex_unlock(&mutex);
        sleep(1); // 1s
        runtime++;
    }
}

static void timerSetTimeout(int fd, uint32_t timeout_s) {
    Connection * current = end;
    pthread_mutex_lock(&mutex);
    while (current != NULL) {
        if (current->fd == fd) {
            current->timeout_s = timeout_s;
            break;
        }
        current = current->prev;
    }
    pthread_mutex_unlock(&mutex);
}

static void timerAddConnection(int fd, uint32_t timeout_s) {
    Connection * connection = (Connection *)malloc(sizeof(Connection));
    connection->fd = fd;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    connection->start_s = ts.tv_sec;

    connection->timeout_s = timeout_s;
    connection->prev = NULL;
    connection->next = NULL;

    if (start == NULL) {
        start = connection;
        end = connection;
    } else {
        pthread_mutex_lock(&mutex);
        end->next = connection;
        connection->prev = end;
        end = connection;
        pthread_mutex_unlock(&mutex);
    }
}

#if defined __cplusplus
}
#endif

#endif
