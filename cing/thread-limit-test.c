#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void * dummy() {
    sleep(5);
}

void * handler_func() {
    printf("REACHED THREAD LIMIT... waited\n");
}

int main() {
    pthread_t threads[100000];
    int ret = 0;
    int i = 0;
    // # detached threads
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    // simulate limit reached
    while (ret == 0) {
        ret = pthread_create(&threads[i++], &attr, dummy, &i);
        if (i == 100000) {
            i = 0;
        }
    }
    pthread_t thread;
    ret = pthread_create(&thread, NULL, handler_func, NULL);
    if (ret == EAGAIN) {
        printf("REACHED THREAD LIMIT, %d... waiting \n", i + 1);
    }
    while (ret == EAGAIN) {
        usleep(1000); // sleep for 1 ms
        ret = pthread_create(&thread, NULL, handler_func, NULL);
    }
    // printf("DONE WAITING?\n");
    pthread_join(thread, NULL);
}