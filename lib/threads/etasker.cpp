#include "etasker.h" 

#include "multithreaded_signals.h"

using namespace WylesLibs;

void ETasker::setThreadTimeout(uint64_t ts) {
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    this->thread_pool_queue[pthread].timeout_s = ts;
    pthread_mutex_unlock(&this->mutex);
}

uint64_t ETasker::getThreadTimeout() {
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    uint64_t timeout_s = this->thread_pool_queue[pthread].timeout_s;
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
    ThreadArg * arg = new ThreadArg(task);
    int ret = pthread_create(&thread, &this->attr, this->threadContext, arg);
    if (ret != 0) {
        std::string msg("Failed to create ETasker thread.");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

void ETasker::timerProcess(void * arg) {
    while(1) {
        pthread_mutex_lock(&this->mutex);
        for (auto i: this->thread_pool) {
            pthread_t pthread = i.first;
            EThread ethread = i.second;
            struct timespec t;
            clock_gettime(CLOCK_MONOTONIC, &t);
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "fd: %d, start: %ld, timeout: %u, current_time: %lu\n", ethread.start_time.tv_sec, ethread.timeout_s, t.tv_sec);
            if (ethread.start_time.tv_sec + ethread.timeout_s <= t.tv_sec) {
                loggerPrintf(LOGGER_DEBUG, "Thread expired sending signal.\n");
                pthread_kill(pthread, SIGKILL);
            }
        }
        pthread_mutex_unlock(&this->mutex);
        sleep(1); // 1s
    }
}

void * ETasker::threadSigAction(int sig, siginfo_t * info, void * context) {
    this->threadTeardown();
}

void * ETasker::threadContext(void * arg) {
    ThreadArg * a = (ThreadArg *)arg;
    // ! IMPORTANT - every application must implement a sig handler that processes SIGKILL.
    WylesLibs::registerThreadSigActionHandler(this->threadSigAction);

    jmp_buf * env = new jmp_buf;
    pthread_t pthread = pthread_self();

    pthread_mutex_lock(&this->mutex);
    this->thread_pool[pthread] = EThread(a->task, ESharedPtr<jmp_buf>(env));
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
    task->initialize();
    task->run();

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
    pthread_mutex_lock(&this->mutex);

    pthread_t pthread = pthread_self();
    EThread ethread = this->thread_pool[pthread];
    ethread.task->onExit();

    // TODO:
    //      if this fails, we don't retry. just the way it is for now.
    if (this->thread_pool_queue.size() > 0) {
        this->thread_pool[pthread].task = nullptr;

        pthread_mutex_unlock(&this->mutex);
        ::longjmp(ESHAREDPTR_GET_REF(ethread.env), 1);
    } else {
        this->thread_pool.erase(pthread);
        pthread_mutex_unlock(&this->mutex);
        // semi-colon; (line-separator)
        pthread_exit();
    }
}