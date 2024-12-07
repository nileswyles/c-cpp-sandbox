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

void ETasker::run(ESharedPtr<ETask> task) {
    pthread_mutex_lock(&this->mutex);
    if (this->thread_limit != SIZE_MAX && this->thread_pool.size() > this->thread_limit) {
        this->thread_pool_queue.push_back(task);
    } else {
        this->taskRun(task);
    }
    pthread_mutex_unlock(&this->mutex);
}

void ETasker::taskRun(ESharedPtr<ETask> task) {
    pthread_t thread;
    ThreadArg<ETasker> * arg = new ThreadArg<ETasker>(this, task);
    int ret = pthread_create(&thread, &this->attr, ETasker::threadContextStatic, arg);
    if (ret != 0) {
        std::string msg("Failed to create ETasker thread.");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

void * ETasker::timerProcess(void * arg) {
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
    if (sig == SIGKILL && pthread == this->timer_thread) {
        pthread_exit(NULL);
    } else {
        this->threadTeardown();
    }
}

void * ETasker::threadContext(void * arg) {
    nice(-14); // let's prioritize this application's threads over other CFS threads in the system...

    ThreadArg<ETasker> * a = static_cast<ThreadArg<ETasker> *>(arg);
    // ! IMPORTANT - every application must implement a sig handler that processes SIGKILL.
    jmp_buf env;
    pthread_t pthread = pthread_self();

    // register sig handler for thread
    pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
    ETasker::thread_specific_sig_handlers[pthread] = this;
    pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

    pthread_mutex_lock(&this->mutex);
    this->thread_pool[pthread] = EThread(a->task, this->initial_timeout_s, ESharedPtr<jmp_buf>(&env));
    pthread_mutex_unlock(&this->mutex);

    delete a; // free before function call, so that it can terminate thread however it wants... 
    ::setjmp(env);

    ESharedPtr<ETask> task = this->thread_pool[pthread].task;
    if (true == task.isNullPtr()) {
        while (this->thread_pool_queue.size() == 0) {} // fixed thread pool loop block while waiting for task.

        pthread_mutex_lock(&this->mutex);
        if (this->thread_pool_queue.size() > 0) {
            this->thread_pool[pthread].task = this->thread_pool_queue.front();
            this->thread_pool_queue.pop_front();
        } else {
            ::longjmp(ESHAREDPTR_GET_REF(this->thread_pool[pthread].env), 1);
        }
        pthread_mutex_unlock(&this->mutex);
    }
    ESHAREDPTR_GET_PTR(task)->initialize();
    ESHAREDPTR_GET_PTR(task)->run();

    if (true == this->fixed) {
        this->thread_pool[pthread].task = nullptr;
        ::longjmp(ESHAREDPTR_GET_REF(this->thread_pool[pthread].env), 1);
    } else {
        // graceful thread teardown
        //  if items are queued, it will loop back to beginning of thread context using longjmp, otherwise it tears down the thread.
        this->threadTeardown();
    }

    return NULL;
}

void ETasker::threadTeardown() {
    pthread_t pthread = pthread_self();

    pthread_mutex_lock(&this->mutex);
    EThread ethread = this->thread_pool[pthread];
    pthread_mutex_unlock(&this->mutex);
    ESHAREDPTR_GET_PTR(ethread.task)->onExit();
    // TODO:
    //      if this fails, we don't retry. just the way it is for now.
    pthread_mutex_lock(&this->mutex);
    if (this->thread_limit != SIZE_MAX && this->thread_pool_queue.size() > 0) {
        this->thread_pool[pthread].task = nullptr;

        pthread_mutex_unlock(&this->mutex);
        ::longjmp(ESHAREDPTR_GET_REF(ethread.env), 1);
    } else {
        pthread_mutex_unlock(&this->mutex); // because deadlocks?

        pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
        ETasker::thread_specific_sig_handlers.erase(pthread);
        pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

        pthread_mutex_lock(&this->mutex);
        this->thread_pool.erase(pthread);
        pthread_mutex_unlock(&this->mutex);
        // semi-colon; (line-separator)
        pthread_exit(NULL);
    }
}