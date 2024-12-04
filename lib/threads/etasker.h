#ifndef WYLESLIBS_ETASKER_H
#define WYLESLIBS_ETASKER_H

// basically an instance of the etimeout class implements existing server connecton stuff
//  but instead of connection it's "task". So there's at least one thread there - decide if this should be one per instance.
//      you will additionally be able to configure a threadpool for use be the task runner (see below)

//      A task contains pointer to associated pthread_t 

//      Then we also have something that I am calling task runner
//          The task runner will either spawn a new thread or add to some queue for the threadpool to process.
//          
//          If no thread pool was configured for this instance, then the taskrunner will throw an exception.
//          
//          The timer will "exit" the task.. (just remove from active tasks list?)
//              In the http case it should close fd and send signal to thread.
//              more generally, we just want to stop the thread.
//              the above implies that an http task includes fd.

//              similarly, the queued tasks will move on to next.
//          queued tasks spawn a new thread when popped from queue.
#include <list>

#include <pthread.h>
#include <time.h>
#include <stdbool.h>

#ifndef LOGGER_ETASKER
#define LOGGER_ETASKER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ETASKER
#include "logger.h"

#define DEFAULT_ETASKER_TIMEOUT_S 7

namespace WylesLibs {
    class ETask {
        public:
            ETask() = default;
            ~ETask() = default;
            virtual void initialize() = 0;
            virtual void run() = 0;
            virtual onExit() = 0;
    };

    class EThread {
        public:
            struct timespec start_time;
            uint64_t timeout_s;
            ESharedPtr<ETask> task;
            EThread(ESharedPtr<ETask> ta, uint32_t ts) {
                task = ta;

                clock_gettime(CLOCK_MONOTONIC, &start_time);
                start_s = t;
                timeout_s = ts;
            }
            ~EThread() = default;
    };

    typedef struct ThreadArg {
        ESharedPtr<task> task;
        std::string thread_id; // or pointer? TODO:
    } ThreadArg;
    
    class ETasker {
        private:
            std::map<std::string, EThread> thread_pool;
            std::list<ESharedPtr<ETask>> thread_pool_queue;
            size_t thread_limit;

            pthread_mutex_t mutex;
            pthread_t timer_thread;
            pthread_attr_t attr;

            void timerProcess(void * arg);
            void * threadSigAction(int sig, siginfo_t * info, void * context);
            void * threadContext(void * arg);
            void taskRun(ESharedPtr<ETask> task);
            void threadTeardown();
        public:
            uint64_t initial_timeout_s;
            ETasker(): ETasker(SIZE_MAX, DEFAULT_ETASKER_TIMEOUT_S) {}
            ETasker(size_t tl): ETasker(tl, DEFAULT_ETASKER_TIMEOUT_S) {
            ETasker(size_t tl, uint64_t ts) {
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, 1);
                thread_limit = tl;
                initial_timeout_s = ts;
                
                pthread_mutex_init(&mutex, nullptr);
                pthread_create(&timer_thread, &attr, timerProcess, NULL);
            } // any other thread paramaters.

            ~ETasker() {
                pthread_kill(&this->timer_thread, SIGKILL);
                pthread_mutex_destroy(&this->mutex);
            }

            void setThreadTimeout(uint32_t ts);
            uint64_t getThreadTimeout();
            void run(ESharedPtr<ETask> task);
    };
};
#endif