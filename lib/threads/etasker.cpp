#include "etasker.h" 

#include "multithreaded_signals.h"

using namespace WylesLibs;

void ETasker::setThreadTimeout(uint32_t ts) {
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    thread_pool_queue[pthread].timeout_s = ts;
    pthread_mutex_unlock(&this->mutex);
}

uint64_t ETasker::getThreadTimeout() {
    // lol...
    pthread_mutex_lock(&this->mutex);
    pthread_t pthread = pthread_self();
    uint64_t timeout_s = thread_pool_queue[pthread].timeout_s;
    pthread_mutex_unlock(&this->mutex);
    return timeout_s;
}

void ETasker::run(ESharedPtr<ETask> task) {
    pthread_mutex_lock(&this->mutex);
    if (thread_limit != SIZE_MAX && thread_pool.size() > thread_limit) {
        thread_pool_queue.push_back(task);
    } else {
        this->taskRun(task);
    }
    pthread_mutex_unlock(&this->mutex);
}

void ETasker::timerProcess(void * arg) {
    while(1) {
        pthread_mutex_lock(&mutex);
        for (auto i: this->thread_pool) {
            EThread ethread = i.second;
            struct timespec t;
            clock_gettime(CLOCK_MONOTONIC, &t);
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "fd: %d, start: %ld, timeout: %u, current_time: %lu\n", ethread.start_time.tv_sec, ethread.timeout_s, t.tv_sec);
            if (ethread.start_time.tv_sec + ethread.timeout_s <= t.tv_sec) {
                loggerPrintf(LOGGER_DEBUG, "Thread expired sending signal.\n");
                raise(SIGKILL);
            }
        }
        pthread_mutex_unlock(&mutex);
        sleep(1); // 1s
    }
}

void * ETasker::threadSigAction(int sig, siginfo_t * info, void * context) {
    this->threadTeardown();
}

void * ETasker::threadContext(void * arg) {
    ThreadArg * a = (ThreadArg *)arg;
    ESharedPtr<ETask> task = a->task;
    std::string thread_id; // or pointer, TODO:
    delete a; // free before function call, so that it can terminate thread however it wants... 

    // ! IMPORTANT - every application must implement a sig handler that processes SIGKILL.
    WylesLibs::registerThreadSigActionHandler(this->threadSigAction);

    pthread_mutex_lock(&mutex);
    thread_pool[pthread_self()] = EThread(task);
    pthread_mutex_unlock(&mutex);

    task->initialize();
    task->run();

    // graceful thread teardown
    this->threadTeardown();

    return NULL;
}

void ETasker::taskRun(ESharedPtr<ETask> task) {
    // TODO: is it okay to share the attr? lol lol
    pthread_t thread;
    ThreadArg * arg = new ThreadArg(thread_id, task);
    int ret = pthread_create(&thread, &this->attr, threadContext, arg);
    if (ret != 0) {
        std::string msg("Failed to create ETasker thread.");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

void ETasker::threadTeardown() {
    pthread_mutex_lock(&this->mutex);

    pthread_t current_pthread = pthread_self();
    EThread ethread = thread_pool[current_pthread];
    ethread.task->onExit();
    thread_pool.erase(current_pthread);

    // TODO:
    //      if this fails, we don't retry. just the way it is for now.
    if (thread_pool_queue.size() > 0) {
        taskRun(thread_pool_queue.pop_front());
    }  

    pthread_mutex_unlock(&this->mutex);

    // semi-colon; (line-separator)
    pthread_exit();
}