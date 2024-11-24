#ifndef WYLESLIBS_FILE_WATCHER_H
#define WYLESLIBS_FILE_WATCHER_H

#include "datastructures/array.h"

#include <poll.h>
#include <sys/inotify.h>
#include <map>
#include <string>

#include <memory>
#include "eshared_ptr.h"

namespace WylesLibs {
    class FileWatcher {
        public:
            std::map<std::string, int> paths_wd_map;
            uint32_t access_mask;
            FileWatcher(SharedArray<std::string> paths, uint32_t mask);
            virtual ~FileWatcher();
            virtual void handle(const struct inotify_event *event) = 0;
            void initialize(ESharedPtr<FileWatcher> ptr);
    };
    extern void fileWatcherThreadStart();
    extern void fileWatcherThreadStop();
};
#endif

