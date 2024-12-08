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

// there are various async-multithreading models
//      the models supported here:
//          1-1 thread per task --- thread is tore down after each task... "unlimited"/os-limited number of concurrent threads
//          many-1 thread per task --- thread is reused if a task is available after current task is finished... limited number of concurrent threads
//              dynamic threads
//                  you can decide whether to teardown if queue is empty or 
//                      this is in some ways an optimization of the 1-1 model.
//                  you can definetly support reuse if a normal teardown... 
//                  it's questionable if you can support reuse if preempted. 
//                  need to see if longjmp, exceptions work in signal action context.
//                  otherwise, you can simply relaunch a thread.
//                  
//                  Continuing on eliminating churn, 
//                      Do we want to wait a while for new task before tearing down thread? in other words, dampen the thread teardown?
//                      when would this be useful?
//                          maybe bursty applications? let's not over-engineer for now.

//              fixed-set threads
//                  keep a fixed-set of threads and poll until new task if queue is empty.
//                  it seems dumb to loop while doing nothing - this doesn't seem valuable.
//                  
//                  an argument to implement thread pools as a fixed-set is to avoid that coupling... so maybe in situations where that coupling is an issue, avoid.
//                  so, maybe allow this option aswell.
//          
//          furthermore, think about some more use-cases
//              think javascript - promises/callbacks
//                  This can be implemented using any of the models outlined above.
//                  task functions are essentially a "callback" but you can build more user-friendly abstractions leveraging that? 
//                  some sort of event loop? where tasks/events are sourced from a single thread, etc. 
//                  no need to restrict to this use-case but useful for conceptualizing (idk)?
//
//          some other models...
//              specific to the socket/fd context.... lol.
//                  considering this model, you might multiplex at each task instead of 1-1 fd to task...
//                  maybe poll if set of fds are available and perform action... this seems overly engineered, complicated and seems to have less performance.
//              

//          key takeaways/principles:
//              1. when thinking about creating useful abstractions, consider application use-cases...
//                  a. process for sort of coming up with an optimal model of abstraction:
//                      1. start at simpilest solution... this will very often offer the most performance.
//                          a. in many cases you can make improvements without affecting performance.
//                      2. some intuition is needed to build the set of possible abstractions/optimizations/improvements - are you optimizing for simplicity, performance, both, etc?
//                      3. once you have the set of possible improvements, choose one.
//                      4. keep in mind what the motivations were for the improvement (simplicity, performance). 
//                          a. how do we characterize these metrics?
//                              a. for performance (and sort of the idea I really wanted to jot down)
//                                     limit time to execute some function?
//                                          limiting the sub functions.
//                                          limit the time to execute sub functions.
//                                     more specifically, in the example of eliminating thread churn (similar to caching policies?)
//                                          thread churn is a function of the invocation curve. in other words, the performance of sub functions depends on when and how often the function is invoked with respect to time..
//                                          alright, so let's define a type of function that is characterized by the above.
//                                              in this example, limiting number of concurrent threads, created a situation where you will need to queue items. Which enabled the optimization of reusing threads instead of tearing down.
//                                                  the need to queue items enabled dynamic threading, adding another dimension of complexity 
//                                                      but in a way that is coupled to how the function is invoked... 
//                                                          it's dependent on a "hidden"/ignored variable.

//                                          what invocation curves with respect to time will benefit from this optimization?
//                                              so, when is thread churned (lol)? timeout and no items queued
//                                                  the invocation curve with respect to time affects th
//                                          
//                      5. will the improvement affect other metrics (not primary motivations for improvement but still needed)
//                          a. decide whether to commit.

//                  I sound like a broken record lol, consolidate this into a notion page. Original thought is important.

//          actionable items:
//              1. many-1 thread per task - 
//                  answer questions surrounding whether exceptions or longjmp work in signal action context. review how exceptions and longjmp work.
//              2. dynamic and fixed thread pooling

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
        private:
            pthread_t pthread;
            std::string msg;
        public:
            ETaskerUnWind(pthread_t pthread): pthread(pthread) {}
            virtual ~ETaskerUnWind() {}
            const char * what() {
                msg = WylesLibs::format("Unwinding stack for thread: {d}", pthread);
                return msg.c_str();
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
            // i'm not finnnaaaa do that LOLOLOLOLOLOLOLOLOL
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
                ETasker * tasker = static_cast<ETasker *>(ptr);
                return tasker->timerProcess(ptr);
            }
            static void * threadContextStatic(void * ptr) {
                ThreadArg<ETasker> * thread_arg = static_cast<ThreadArg<ETasker> *>(ptr);
                return thread_arg->tasker->threadContext(ptr);
            }
            static pthread_mutex_t thread_specific_sig_handler_mutex;
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
            ETasker(size_t tl, uint64_t ts, bool fixed): ETasker(tl, ts, fixed, PTHREAD_STACK_MIN) {}
            ETasker(size_t tl, uint64_t timeout_s, bool fixed, size_t stack_size) {
                thread_limit = tl;
                initial_timeout_s = timeout_s;
                num_threads = 0;
                
                pthread_mutex_init(&mutex, nullptr);
                fixed = fixed;
                new_thread_stack_size = stack_size;

                // intialize timer
                ETasker::setTimerThreadAttributes(&timer_attr);
                pthread_create(&this->timer_thread, &timer_attr, ETasker::timerProcessStatic, this);
                pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
                ETasker::thread_specific_sig_handlers[this->timer_thread] = this;
                pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

                if (true == fixed) {
                    int nprocs = get_nprocs();
                    if (tl > static_cast<size_t>(nprocs)) {
                        std::string msg = WylesLibs::format("Fixed thread pool cannot be of larger than nprocs {d}", nprocs);
                        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                }
            } // any other thread paramaters.

            ~ETasker() {
                // pthread_kill(this->timer_thread, SIGKILL);
                pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
                ETasker::thread_specific_sig_handlers.erase(this->timer_thread);
                pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);
                pthread_mutex_destroy(&this->mutex);
            }

            void setThreadTimeout(uint64_t ts);
            uint64_t getThreadTimeout();
            void threadSigAction(int sig, siginfo_t * info, void * context);
            int run(ESharedPtr<ETask> task);
    };
};
#endif