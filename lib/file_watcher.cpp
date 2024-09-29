#include "file_watcher.h"
#include <unistd.h>

#include <pthread.h>
#include <memory>

using namespace WylesLibs;

static std::map<int, std::weak_ptr<FileWatcher>> registeredWatchers{};
static pthread_t watcher_thread;
static bool thread_run;
static int fd = -1;

// TODO: use std::mutex instead of pthread_mutex? also std::thread? probably not... inotify is linux only anyways...
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

FileWatcher::FileWatcher(Array<std::string> paths, uint32_t access_mask) {
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
        int wd = inotify_add_watch(fd, w.first.c_str(), access_mask);
        if (wd == -1) {
            throw std::runtime_error("Cannot watch path: " + w.first);
        }
        paths_wd_map[w.first] = wd;
        registeredWatchers[wd] = ptr;
    }
    pthread_mutex_unlock(&mutex);
}

static void * watcherRun(void * arg) {
    while (thread_run) {
        pthread_mutex_lock(&mutex);
        /* Some systems cannot read integer variables if they are not
           properly aligned. On other systems, incorrect alignment may
           decrease performance. Hence, the buffer used for reading from
           the inotify file descriptor should have the same alignment as
           struct inotify_event. */
        char buf[4096]
            __attribute__ ((aligned(__alignof__(struct inotify_event))));
        const struct inotify_event *event;
        ssize_t len;
        /* Loop while events can be read from inotify file descriptor. */
        for (;;) {
            /* Read some events. */
            len = read(fd, buf, sizeof(buf));
            if (len <= 0) {
                if (errno != EAGAIN) {
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Failed to read from inotify fd: %d\n", fd);
                    fileWatcherThreadStop();
                }
                break;
            }

            /* Loop over all events in the buffer. */
            for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                event = (const struct inotify_event *) ptr;
                registeredWatchers[event->wd].lock()->handle(event);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    return nullptr;
}

extern void WylesLibs::fileWatcherThreadStart() {
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
}

extern void WylesLibs::fileWatcherThreadStop() {
    for (auto w: registeredWatchers) {
        w.second.lock()->paths_wd_map.clear();
        inotify_rm_watch(fd, w.first);
    }
    registeredWatchers.clear(); // should call destructors right?
    close(fd);
    fd = -1;
    thread_run = false;
}