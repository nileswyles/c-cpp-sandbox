#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/resource.h>
#include <errno.h>


void * do_stuff(void * args) {
    int thread_id = *((int *)args);
    printf("THREAD ID STARTED: %d\n", thread_id);
    sleep(30);
    printf("THREAD ID FINISHED: %d\n", thread_id);
}

#define NUM_THREADS 28000
int main(int argc, char* argv[]) {
    pthread_t thread[NUM_THREADS];
    int thread_args[NUM_THREADS];
    struct rlimit old = {};
    getrlimit(RLIMIT_NPROC, &old);
    printf("old: %ld, %ld\n", old.rlim_cur, old.rlim_max);
    struct rlimit new = {
        .rlim_cur = 1000000,
        .rlim_max = 1000000,
    };
    setrlimit(RLIMIT_NPROC, &new);
    getrlimit(RLIMIT_NPROC, &old);
    printf("new: %ld, %ld\n", old.rlim_cur, old.rlim_max);

    printf("CREATING THREADS\n");
    int result_code;
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i] = i;
        result_code = pthread_create(&thread[i], NULL, do_stuff, &thread_args[i]);
        printf("THREAD WITH RESULT CODE: %d, %d\n", i, result_code);
        assert(!result_code);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        result_code = pthread_join(thread[i], NULL);
        assert(!result_code);
    }

    printf("THREADS FINISHED\n");
    printf("%d, %s", argc, argv[0]);
    return 0;
}