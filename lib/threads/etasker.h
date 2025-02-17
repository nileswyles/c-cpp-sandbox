#ifndef WYLESLIBS_ETASKER_H
#define WYLESLIBS_ETASKER_H

#include "memory/pointers.h"
#include "string_format.h"

#include <list>
#include <map>

#include <stddef.h>
#include <setjmp.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <sys/sysinfo.h>
#include <signal.h>

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
            virtual ~ETask() = default;
            virtual void initialize() = 0;
            virtual void run() = 0;
            virtual void onExit() = 0;
    };

    class ETaskerUnWind: public std::exception {
        public:
            ETaskerUnWind() {}
            virtual ~ETaskerUnWind() {}
            const char * what() {
                return nullptr;
            }
            ETaskerUnWind(const ETaskerUnWind&) = default;
            ETaskerUnWind& operator=(const ETaskerUnWind&) = default;
            ETaskerUnWind(ETaskerUnWind&&) = default;
            ETaskerUnWind& operator=(ETaskerUnWind&&) = default;
    };

    class EThread {
        public:
            struct timespec start_time;
            uint64_t timeout_s;
            ESharedPtr<ETask> task;
            jmp_buf * env; // ! IMPORTANT - since queuing is a common and exceptions are expensive, longjmp and setjmp to return back to thread context from signal preemption.
            pthread_t * pthread;
            pthread_attr_t * pthread_attr;

            EThread() = default;
            EThread(ESharedPtr<ETask> ta, uint64_t ts, jmp_buf * env, pthread_t * thread, pthread_attr_t * attr) {
                task = ta;

                clock_gettime(CLOCK_MONOTONIC, &start_time);
                timeout_s = ts;
                env = env;
                pthread = thread;
                pthread_attr = attr;
            }
            ~EThread() {}
    };

    template<typename T>
    class ThreadArg {
        public:
            // as opposed to casting to void? Is there a reason not too? other than ...
            T * tasker;
            ESharedPtr<ETask> task;
            pthread_t * pthread;
            pthread_attr_t * pthread_attr;
            ThreadArg(T * tasker, ESharedPtr<ETask> task, pthread_t * pthread, pthread_attr_t * pthread_attr): 
                tasker(tasker), task(task), pthread(pthread), pthread_attr(pthread_attr) {}
            ~ThreadArg() = default;
    };

    class ETasker {
        private:
            std::map<pthread_t, EThread> thread_pool;
            std::list<ESharedPtr<ETask>> thread_pool_queue;
            size_t thread_limit;
            size_t num_threads;

            pthread_mutex_t mutex;
            pthread_t timer_thread; 
            // IMPORTANT - as of 2024, this is defined as a "thread identifier", I expect it to be unique and comparable-type.
            pthread_attr_t timer_attr;
            bool fixed;
            size_t new_thread_stack_size;
            
            static void setThreadAttributes(pthread_attr_t * attr, size_t stack_size) {
                int i = 0;
                i = pthread_attr_init(attr); // ! IMPORTANT - pthread_create arg is const.
                if (i != 0) {
                    std::string msg("Failed to init ETasker thread attr.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread attr detach state to PTHREAD_CREATE_DETACHED.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread inherit state to PTHREAD_EXPLICIT_SCHED.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setschedpolicy(attr, SCHED_OTHER); // OTHER is the default? CFS?
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread sched policy to SCHED_OTHER.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                struct sched_param param = {
                    .sched_priority = 0
                };
                i = pthread_attr_setschedparam(attr, &param);
                if (i != 0) {
                    std::string msg = WylesLibs::format("Failed to set ETasker thread sched param - sched priority to {d}.", param.sched_priority);
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setstacksize(attr, stack_size);
                if (i != 0) {
                    std::string msg = WylesLibs::format("Failed to set ETasker thread stack size to: {d}", stack_size);
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread scope to PTHREAD_SCOPE_SYSTEM.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
            }
            static void setTimerThreadAttributes(pthread_attr_t * attr) {
                int i = 0;
                i = pthread_attr_init(attr); // ! IMPORTANT - pthread_create arg is const.
                if (i != 0) {
                    std::string msg("Failed to init ETasker thread attr.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread attr detach state to PTHREAD_CREATE_DETACHED.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread inherit state to PTHREAD_EXPLICIT_SCHED.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setschedpolicy(attr, SCHED_FIFO);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread sched policy to SCHED_FIFO.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                struct sched_param param = {
                    .sched_priority = 99
                };
                i = pthread_attr_setschedparam(attr, &param);
                if (i != 0) {
                    std::string msg = WylesLibs::format("Failed to set ETasker thread sched param - sched priority to {d}.", param.sched_priority);
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setstacksize(attr, PTHREAD_STACK_MIN);
                if (i != 0) {
                    std::string msg = WylesLibs::format("Failed to set ETasker thread stack size to: {d}", PTHREAD_STACK_MIN);
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
                i = pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
                if (i != 0) {
                    std::string msg("Failed to set ETasker thread scope to PTHREAD_SCOPE_SYSTEM.");
                    loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                }
            }
            static void logThreadAttributes(pthread_attr_t * attr) {
                int s, i;
                size_t v;
                void *stack_addr;
                struct sched_param sp;
                s = pthread_attr_getdetachstate(attr, &i);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getdetachstate.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Detach state        = %s\n",
                       (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
                       (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
                       "???");

                s = pthread_attr_getscope(attr, &i);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getscope.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Scope               = %s\n",
                       (i == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
                       (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
                       "???");

                s = pthread_attr_getinheritsched(attr, &i);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getinheritsched.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Inherit scheduler   = %s\n",
                       (i == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
                       (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
                       "???"
                );

                s = pthread_attr_getschedpolicy(attr, &i);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getschedpolicy.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Scheduling policy   = %s\n",
                       (i == SCHED_OTHER) ? "SCHED_OTHER" :
                       (i == SCHED_FIFO)  ? "SCHED_FIFO" :
                       (i == SCHED_RR)    ? "SCHED_RR" :
                       "???"
                );

                s = pthread_attr_getschedparam(attr, &sp);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getschedparam.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Scheduling priority = %d\n", sp.sched_priority);

                s = pthread_attr_getguardsize(attr, &v);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getguardsize.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Guard size          = %zu bytes\n", v);

                s = pthread_attr_getstack(attr, &stack_addr, &v);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getstack.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Stack address       = %p\n", stack_addr);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Stack size          = %#zx bytes\n", v);

                s = pthread_attr_getstacksize(attr, &v);
                if (s != 0)
                    std::runtime_error("Failed to log pthread_attr_getstack.");
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Stack size          = %zu bytes\n", v);
            }
            static void * timerProcessStatic(void * ptr) {
                sleep(1); // lol...
                ETasker * tasker = static_cast<ETasker *>(ptr);
                return tasker->timerProcess(ptr);
            }
            static void * threadContextStatic(void * ptr) {
                ThreadArg<ETasker> * thread_arg = static_cast<ThreadArg<ETasker> *>(ptr);
                return thread_arg->tasker->threadContext(ptr);
            }
            static pthread_mutex_t thread_specific_sig_handler_mutex;
            bool hasThreads();
            int taskRun(ESharedPtr<ETask> task);
            void * timerProcess(void * arg);
            void * threadContext(void * arg);
            void threadTeardown(bool force);
            void processThread();
        public:
            static std::map<pthread_t, ETasker *> thread_specific_sig_handlers;
            uint64_t initial_timeout_s;

            ETasker(): ETasker(SIZE_MAX, DEFAULT_ETASKER_TIMEOUT_S, false) {}
            ETasker(size_t tl): ETasker(tl, DEFAULT_ETASKER_TIMEOUT_S, false) {}
            ETasker(size_t tl, uint64_t ts): ETasker(tl, ts, false, PTHREAD_STACK_MIN) {}
            ETasker(size_t tl, uint64_t ts, bool fixed): ETasker(tl, ts, fixed, PTHREAD_STACK_MIN) {}
            ETasker(size_t tl, uint64_t timeout_s, bool fixed, size_t stack_size) {
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "ETasker initilaized with the following parameters:\n\tthread_limit: %lu, timeout_s: %lu, fixed: %s, stack_size: %lu\n", tl, timeout_s, fixed ? "true" : "false", stack_size);
                thread_limit = tl;
                initial_timeout_s = timeout_s;
                num_threads = 0;
                
                pthread_mutex_init(&mutex, nullptr);
                fixed = fixed;
                new_thread_stack_size = stack_size;

                // intialize timer
                ETasker::setTimerThreadAttributes(&timer_attr);
                pthread_create(&timer_thread, &timer_attr, ETasker::timerProcessStatic, this);

                if (true == fixed) {
                    int nprocs = get_nprocs();
                    if (tl > static_cast<size_t>(nprocs)) {
                        std::string msg = WylesLibs::format("Fixed thread pool cannot be of larger than nprocs {d}", nprocs);
                        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                }
            } // any other thread paramaters.

            ~ETasker() noexcept(false) {
                pthread_sigqueue(this->timer_thread, SIGTERM, {0});
                pthread_mutex_lock(&this->mutex);
                for (auto i: this->thread_pool) {
                    loggerPrintf(LOGGER_DEBUG, "Thread expired sending signal.\n");
                    pthread_t pthread = i.first;
                    pthread_sigqueue(pthread, SIGTERM, {0});
                }
                pthread_mutex_unlock(&this->mutex);
                size_t teardown_timeout_s = 7;
                while (teardown_timeout_s > 0 && true == this->hasThreads()) {
                    sleep(1);
                    teardown_timeout_s--;
                }
                if (teardown_timeout_s == 0 && true == this->hasThreads()) {
                    std::string msg("Failed to teardown threads within specified timeout.");
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    sleep(1); // give enough time for pthread_exit to finish on all threads.
                }
                pthread_mutex_destroy(&this->mutex);
            }

            void setThreadTimeout(uint64_t ts);
            uint64_t getThreadTimeout();
            void threadSigAction(int sig, siginfo_t * info, void * context);
            int run(ESharedPtr<ETask> task);
            size_t threads() {
                return num_threads;
            }
    };
};
#endif