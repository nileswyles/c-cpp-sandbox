#ifndef WYLESLIBS_SERVER_CONNECTION_TIMER_H
#define WYLESLIBS_SERVER_CONNECTION_TIMER_H

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
#include <stdbool.h>

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

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t process_thread;

static bool run = false;

static void closeConnection(Connection * connection);
static int timerStart();
static void timerStop();
static void * timerProcess(void * arg);
static void timerSetTimeout(int fd, uint32_t timeout_s);
static uint32_t timerGetTimeout(int fd);
static void timerAddConnection(int fd, uint32_t timeout_s);
static void timerRemoveConnection(int fd);
static void removeConnection(Connection * connection);

static void closeConnection(Connection * connection) {
    loggerPrintf(LOGGER_DEBUG, "Closing connection (with FD: %d) due to timer expiration.\n", connection->fd);
    close(connection->fd);
    removeConnection(connection);
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

static void * timerProcess(void * arg) {
    while(run) {
        pthread_mutex_lock(&mutex);
        Connection * current = start;
        while (current != NULL) {
            Connection * next = current->next;
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "fd: %d, start: %lu, timeout: %u, current_time: %lu\n", current->fd, current->start_s, current->timeout_s, ts.tv_sec);
            if (current->start_s + current->timeout_s <= ts.tv_sec) {
                closeConnection(current);
            }
            current = next;
        }
        pthread_mutex_unlock(&mutex);
        sleep(1); // 1s
    }

    return NULL;
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

static uint32_t timerGetTimeout(int fd) {
    Connection * current = end;
    pthread_mutex_lock(&mutex);
    while (current != NULL) {
        if (current->fd == fd) {
            return current->timeout_s;
        }
        current = current->prev;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
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

static void timerRemoveConnection(int fd) {
    pthread_mutex_lock(&mutex);
    Connection * current = start;
    while (current != NULL) {
        if (current->fd == fd) {
            removeConnection(current);
            break;
        } else {
            current = current->next;
        }
    }
    pthread_mutex_unlock(&mutex);
}

static void removeConnection(Connection * connection) {
    Connection * prev = connection->prev;
    Connection * next = connection->next;
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
    free(connection);
}

#if defined __cplusplus
}
#endif

#endif
