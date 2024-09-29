#include "file_watcher.h"
#include <unistd.h>

using namespace WylesLibs;

static std::map<int, FileWatcher *> registeredWatchers{};
static pthread_t watcher_thread;
static bool thread_run;
static pollfd poll_fd = {.fd = -1};

FileWatcher::FileWatcher(Array<std::string> paths, uint32_t access_mask) {
    if (poll_fd.fd == -1) {
        std::runtime_error("File watcher thread has not been initialized!");
    }

    size_t watch_descriptor_size = paths.size();
    for (size_t i = 0; i < watch_descriptor_size; i++) {
        int wd = inotify_add_watch(poll_fd.fd, paths.at(i).c_str(), access_mask);
        if (wd == -1) {
            throw std::runtime_error("Cannot watch path: " + paths[i]);
        }
        paths_wd_map[paths[i]] = wd;
    }
}

static void watcherRun(void * arg) {
    int poll_num;
    /* Wait for events and/or terminal input. */
    printf("Listening for events.\n");
    while (thread_run) {
        poll_num = poll(&poll_fd, 1, -1);
        if (poll_num == -1) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "File watcher poll failed.\n");
        }
        if (poll_num > 0) {
            if (poll_fd.revents & POLLIN) {

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
                len = read(poll_fd.fd, buf, sizeof(buf));
                if (len == -1 && errno != EAGAIN) {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                /* If the nonblocking read() found no events to read, then
                   it returns -1 with errno set to EAGAIN. In that case,
                   we exit the loop. */
                if (len <= 0)
                    break;
                /* Loop over all events in the buffer. */
                for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
                    event = (const struct inotify_event *) ptr;
                    registeredWatchers[event->wd]->handle(event);
                }
            }
        }
    }
}

extern void WylesLibs::fileWatcherThreadStart() {
    /* Create the file descriptor for accessing the inotify API. */
    int fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1) {
        throw std::runtime_error("inotify_init1");
    }
    poll_fd = {.fd = fd, .events = POLLIN};

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);

    thread_run = true;
    pthread_create(&watcher_thread, &attr, watcherRun, nullptr);
}

extern void WylesLibs::fileWatcherRegister(FileWatcher * watcher);
    for (auto w: watcher->paths_wd_map) {
        registeredWatchers[w.second] = watcher;
    }
}

extern void WylesLibs::fileWatcherUnregister(FileWatcher * watcher) {
    for (auto w: watcher->paths_wd_map) {
        registeredWatchers.erase(w.second);
    }
}

extern void WylesLibs::fileWatcherThreadStop() {
    registeredWatchers.clear();
    close(poll_fd.fd);
    poll_fd.fd = -1;
    thread_run = false;
}