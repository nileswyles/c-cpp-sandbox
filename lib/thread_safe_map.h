#ifndef WYLESLIBS_THREAD_SAFE_MAP_H
#define WYLESLIBS_THREAD_SAFE_MAP_H

template<typename T, typename L>
class ThreadSafeMap: public std::map<T, L> {
    private:
        pthread_mutex_t mutex;
    public:
        // TODO: think about whether I want to atomicize (atomize?), the operations specific to map or also related operations?
        //   can I support both? Do I need multiple mutexes?
        ThreadSafeMap(): std::map<T, L>() {
            pthread_mutex_init(&mutex);
        }
        ~ThreadSafeMap() {
            pthread_mutex_destroy(&mutex);
        }
        pthread_mutex_t * getMutex() {
            return &mutex;
        };
};
#endif