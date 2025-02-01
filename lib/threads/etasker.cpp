#include "etasker.h" 

#include <stdio.h>
#include <unistd.h>

using namespace WylesLibs;

pthread_mutex_t ETasker::thread_specific_sig_handler_mutex = PTHREAD_MUTEX_INITIALIZER;
std::map<pthread_t, ETasker *> ETasker::thread_specific_sig_handlers = std::map<pthread_t, ETasker *>();

void ETasker::setThreadTimeout(uint64_t ts) {
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    EThread ethread = this->thread_pool.at(pthread);
    ethread.timeout_s = ts;
    this->thread_pool[pthread] = ethread; // std::move?
    pthread_mutex_unlock(&this->mutex);
}

uint64_t ETasker::getThreadTimeout() {
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    EThread ethread = this->thread_pool.at(pthread);
    uint64_t timeout_s = ethread.timeout_s;
    pthread_mutex_unlock(&this->mutex);
    return timeout_s;
}

int ETasker::run(ESharedPtr<ETask> task) {
    int ret = 0;
    pthread_mutex_lock(&this->mutex);
    if (this->thread_limit != SIZE_MAX && this->num_threads >= this->thread_limit) {
        this->thread_pool_queue.push_back(task);
    } else {
        ret = this->taskRun(task);
        if (ret == 0) {
            this->num_threads++;
        }
    }
    pthread_mutex_unlock(&this->mutex);
    return ret;
}

int ETasker::taskRun(ESharedPtr<ETask> task) {
    // this is most definetly not necessary but to demonstrate cool club behaviour? I think it's implied these are reallocated by pthread_create?
    pthread_t * thread = new pthread_t;
    pthread_attr_t * attr = new pthread_attr_t;
    ThreadArg<ETasker> * arg = new ThreadArg<ETasker>(this, task, thread, attr);
    ETasker::setThreadAttributes(attr, this->new_thread_stack_size);
    return pthread_create(thread, attr, ETasker::threadContextStatic, arg);
}

void * ETasker::timerProcess(void * arg) {
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Logging Thread Atrributes for the ETasker Timer thread: \n");
    ETasker::logThreadAttributes(&this->timer_attr);

    pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
    ETasker::thread_specific_sig_handlers[timer_thread] = this;
    pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

    while(1) {
        pthread_mutex_lock(&this->mutex);
        for (auto i: this->thread_pool) {
            pthread_t pthread = i.first;
            EThread ethread = i.second;
            struct timespec t;
            clock_gettime(CLOCK_MONOTONIC, &t);
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "start: %ld, timeout: %lu, current_time: %lu\n", ethread.start_time.tv_sec, ethread.timeout_s, t.tv_sec);
            if (static_cast<uint64_t>(ethread.start_time.tv_sec) + ethread.timeout_s <= static_cast<uint64_t>(t.tv_sec)) {
                loggerPrintf(LOGGER_DEBUG, "Thread expired sending signal.\n");
                pthread_kill(pthread, SIGKILL);
            }
        }
        pthread_mutex_unlock(&this->mutex);
        sleep(4); // thread priority is set to the maximum so decided +-4 seconds to further free up the core for other processes in the system.
    }
}

void ETasker::threadSigAction(int sig, siginfo_t * info, void * context) {
    pthread_t pthread = pthread_self();
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "sig action: %d for pthread %ld\n", sig, pthread);
    if (sig == SIGKILL && pthread == this->timer_thread) {
        pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
        ETasker::thread_specific_sig_handlers.erase(this->timer_thread);
        pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);
        pthread_exit(NULL);
    } else {
        this->threadTeardown(false);
    }
}

void * ETasker::threadContext(void * arg) {
    nice(-14); 
    // let's prioritize this application's stalled threads over other CFS stalled threads in the system...
    //  these threads should be relatively short-lived compared to websocket upgrade and long uploads so these stalled threads are prioritized.
    //  currently (thread creation time being equal), it's 
    //    short lived api threads -> endless websockets (upgrades!) -> long but not endless uploads
    //  optimize for api latency? the more common case.

    // TODO: should this stay this way? - I think it just matters that it has some nice value (very stalled tasks are prioritized over newer tasks)

    ThreadArg<ETasker> * a = static_cast<ThreadArg<ETasker> *>(arg);
    // ! IMPORTANT - every application must implement a sig handler that processes SIGKILL.
    jmp_buf env;
    pthread_t pthread = pthread_self();

    // register sig handler for thread
    pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
    ETasker::thread_specific_sig_handlers[pthread] = this;
    pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

    pthread_mutex_lock(&this->mutex);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Logging Thread Atrributes for ETasker created threads: \n");
    ETasker::logThreadAttributes(a->pthread_attr);
    this->thread_pool[pthread] = EThread(a->task, this->initial_timeout_s, &env, a->pthread, a->pthread_attr);
    pthread_mutex_unlock(&this->mutex);

    delete a; // free before function call, so that it can terminate thread however it wants... 
    ::setjmp(env);

    bool terminate = false;
    while (1) {
        try {
            processThread();
        } catch (ETaskerUnWind& e) {
            // unwound stack continue normally
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Unwound stack for this thread: %ld.\n", pthread);
        } catch (std::exception& e) {
            loggerPrintf(LOGGER_INFO, "%s\n", e.what());
            loggerPrintf(LOGGER_INFO, "Caught exception terminating thread.\n");

            terminate = true;
        }
        if (true == terminate) {
            // to ensure exception object is deallocated? it says not globally allocated but does that imply it's placed on the stack lololol?
            //      VAGINA
            this->threadTeardown(true);
        }
    }

    return NULL;
}

void ETasker::processThread() {
    pthread_t pthread = pthread_self();
    while (true == this->thread_pool[pthread].task.isNullPtr()) {
        while (this->thread_pool_queue.size() == 0) {} // fixed thread pool loop block while waiting for task.

        pthread_mutex_lock(&this->mutex);
        if (this->thread_pool_queue.size() > 0) {
            this->thread_pool[pthread].task = this->thread_pool_queue.front();
            this->thread_pool_queue.pop_front();
        }
        pthread_mutex_unlock(&this->mutex);
    }

    ESharedPtr<ETask> task = this->thread_pool[pthread].task;
    ESHAREDPTR_GET_PTR(task)->initialize();
    ESHAREDPTR_GET_PTR(task)->run();

    if (true == this->fixed) {
        this->thread_pool[pthread].task = nullptr;
        // clear task variable for when this function is called again in main loop
    } else {
        // graceful thread teardown
        //  if items are queued, it will loop back to beginning of thread context using longjmp, otherwise it tears down the thread.
        this->threadTeardown(false);
    }
}

void ETasker::threadTeardown(bool force) {
    pthread_t pthread = pthread_self();

    pthread_mutex_lock(&this->mutex);
    EThread ethread = this->thread_pool[pthread];
    pthread_mutex_unlock(&this->mutex);
    try {
        if (false == ethread.task.isNullPtr()) {
            ESHAREDPTR_GET_PTR(ethread.task)->onExit();
        }
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_INFO, "%s\n", e.what());
        loggerPrintf(LOGGER_INFO, "Caught exception in task::onExit. Please review your code there may be a leak!\n");
    }

    pthread_mutex_lock(&this->mutex);
    if (this->thread_limit != SIZE_MAX && this->thread_pool_queue.size() > 0 && false == force) {
        this->thread_pool[pthread].task = nullptr;

        pthread_mutex_unlock(&this->mutex);
#ifdef WYLESLIBS_ETASKER_LONGJMP
        ::longjmp(*(ethread.env), 1);
#else
        // TODO: read more about performance impact of this... 
        //      I'm assuming it's less than creating a new thread and comperable to longjmp. Only difference is an extra allocation (of the exception object) since there's only one try catch up the stack?
        throw ETaskerUnWind();
#endif
    } else {
        pthread_mutex_unlock(&this->mutex); // because deadlocks?

        pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
        // ! IMPORTANT - we don't want dangling ptrs
        ETasker::thread_specific_sig_handlers.erase(pthread);
        pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

        pthread_mutex_lock(&this->mutex);
        // jumping through hoops to avoid shared ptr? LOLOLOLOL
        //      still not 100%-fool proof, it is assumed pthread_exit doesn't require this pthread info... 
        delete ethread.pthread;
        delete ethread.pthread_attr;
        this->thread_pool.erase(pthread);
        this->num_threads--;
        pthread_mutex_unlock(&this->mutex);
        // semi-colon; (line-separator)
        pthread_exit(NULL);
    }
}