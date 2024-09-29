#ifndef WYLESLIBS_FILE_WATCHER_H
#define WYLESLIBS_FILE_WATCHER_H

#include "array.h"

#include <poll.h>
#include <sys/inotify.h>
#include <map>
#include <string>

namespace WylesLibs {
    class FileWatcher {
        public:
            std::map<std::string, int> paths_wd_map;
            FileWatcher(Array<std::string> paths, uint32_t access_mask);
            virtual void handle(const struct inotify_event *event) = 0;
    };
    extern void fileWatcherThreadStart();
    extern void fileWatcherRegister(FileWatcher * watcher);
    extern void fileWatcherUnregister(FileWatcher * watcher);
    extern void fileWatcherThreadStop();
};
#endif

