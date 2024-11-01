#include "file/file_watcher.h"
#include <unistd.h>

#include <pthread.h>
#include <memory>

using namespace WylesLibs;

// TODO: log toggles.

static std::map<int, std::weak_ptr<FileWatcher>> registeredWatchers{};
static pthread_t watcher_thread;
static bool thread_run;
static int fd = -1;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

FileWatcher::FileWatcher(SharedArray<std::string> paths, uint32_t access_mask) {
    if (fd == -1) {
        std::runtime_error("File watcher thread has not been initialized!");
    }
    size_t watch_descriptor_size = paths.size();
    for (size_t i = 0; i < watch_descriptor_size; i++) {
        paths_wd_map[paths[i]] = -1;
    }
    access_mask = access_mask;
}

FileWatcher::~FileWatcher() {
    pthread_mutex_lock(&mutex);
    for (auto w: this->paths_wd_map) {
        registeredWatchers.erase(w.second);
        inotify_rm_watch(fd, w.second);
    }
    paths_wd_map.clear();
    pthread_mutex_unlock(&mutex);
}

void FileWatcher::initialize(std::shared_ptr<FileWatcher> ptr) {
    pthread_mutex_lock(&mutex);
    for (auto w: this->paths_wd_map) {
        // TODO:
        // okay, so this needs absolute paths?
        // std::string path = "/workspaces/c-cpp-sandbox/http_test/" + w.first;
        std::string path = w.first;
        int wd = inotify_add_watch(fd, path.c_str(), IN_CREATE);
        if (wd == -1) {
            throw std::runtime_error("Cannot watch path: " + path);
        }
        paths_wd_map[w.first] = wd;
        registeredWatchers[wd] = ptr;
    }
    pthread_mutex_unlock(&mutex);
}

#define FILE_WATCHER_SIZE sizeof(struct inotify_event) * 270

static void * watcherRun(void * arg) {
    char buf[FILE_WATCHER_SIZE];
    const struct inotify_event *event;
    ssize_t len;
    try {
        /* Loop while events can be read from inotify file descriptor. */
        bool loop = thread_run;
        while (true == loop) {
            pthread_mutex_lock(&mutex);
            /* Read some events. */
            len = read(fd, buf, sizeof(buf));
            if (len <= 0) {
                if (errno != EAGAIN) {
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Failed to read from inotify fd: %d\n", fd);
                    throw std::runtime_error("Failed to read from inotify fd.");
                }
                continue;
            }
    
            /* Loop over all events in the buffer. */
            for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event *) ptr;
                if (auto watcher = registeredWatchers[event->wd].lock()) {
                    watcher->handle(event);
                }
            }
            loop = thread_run;
            pthread_mutex_unlock(&mutex);
        }
    } catch(const std::exception& e) {
        pthread_mutex_unlock(&mutex);
        loggerPrintf(LOGGER_DEBUG, "Exception thrown proccessing FileWatcher->handle: '%s'", e.what());
        fileWatcherThreadStop();
    }
    return nullptr;
}

extern void WylesLibs::fileWatcherThreadStart() {
    pthread_mutex_lock(&mutex);

    /* Create the file descriptor for accessing the inotify API. */
    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        throw std::runtime_error("inotify_init1");
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);

    thread_run = true;
    pthread_create(&watcher_thread, &attr, watcherRun, nullptr);

    pthread_mutex_unlock(&mutex);
}

extern void WylesLibs::fileWatcherThreadStop() {
    pthread_mutex_lock(&mutex);

    for (auto w: registeredWatchers) {
        w.second.lock()->paths_wd_map.clear();
        inotify_rm_watch(fd, w.first);
    }
    registeredWatchers.clear(); // should call destructors right?
    close(fd);
    fd = -1;
    thread_run = false;

    pthread_mutex_lock(&mutex);
}