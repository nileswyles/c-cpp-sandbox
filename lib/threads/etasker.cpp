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
    if (this->thread_limit != SIZE_MAX && this->thread_pool.size() > this->thread_limit) {
        this->thread_pool_queue.push_back(task);
    } else {
        ret = this->taskRun(task);
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

    ESharedPtr<ETask> task = this->thread_pool[pthread].task;
    if (true == task.isNullPtr()) {
        while (this->thread_pool_queue.size() == 0) {} // fixed thread pool loop block while waiting for task.

        pthread_mutex_lock(&this->mutex);
        if (this->thread_pool_queue.size() > 0) {
            this->thread_pool[pthread].task = this->thread_pool_queue.front();
            this->thread_pool_queue.pop_front();
        } else {
            ::longjmp(*(this->thread_pool[pthread].env), 1);
        }
        pthread_mutex_unlock(&this->mutex);
    }
    ESHAREDPTR_GET_PTR(task)->initialize();
    ESHAREDPTR_GET_PTR(task)->run();

    if (true == this->fixed) {
        this->thread_pool[pthread].task = nullptr;
        ::longjmp(*(this->thread_pool[pthread].env), 1);
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

    pthread_mutex_lock(&this->mutex);
    if (this->thread_limit != SIZE_MAX && this->thread_pool_queue.size() > 0) {
        this->thread_pool[pthread].task = nullptr;

        pthread_mutex_unlock(&this->mutex);
        ::longjmp(*(ethread.env), 1);
    } else {
        pthread_mutex_unlock(&this->mutex); // because deadlocks?

        pthread_mutex_lock(&ETasker::thread_specific_sig_handler_mutex);
        ETasker::thread_specific_sig_handlers.erase(pthread);
        pthread_mutex_unlock(&ETasker::thread_specific_sig_handler_mutex);

        pthread_mutex_lock(&this->mutex);
        // jumping through hoops to avoid shared ptr? LOLOLOLOL
        //      still not 100%-fool proof, it is assumed pthread_exit doesn't require this pthread info... 
        delete ethread.pthread;
        delete ethread.pthread_attr;
        this->thread_pool.erase(pthread);
        pthread_mutex_unlock(&this->mutex);
        // semi-colon; (line-separator)
        pthread_exit(NULL);
    }
}