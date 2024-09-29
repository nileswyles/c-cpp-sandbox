#ifndef WYLESLIBS_HTTP_FILE_WATCHER_H
#define WYLESLIBS_HTTP_FILE_WATCHER_H

#include "file_watcher.h"
#include "array.h"
#include "web/http/config.h"

#include <poll.h>
#include <sys/inotify.h>
#include <unordered_map>
#include <string>

using namespace WylesLibs;

namespace WylesLibs::Http {
    class HttpFileWatcher: public FileWatcher {
        public:
            HttpServerConfig config;
            std::unordered_map<std::string, std::string> * static_paths;
            pthread_mutex_t * static_paths_mutex;

            HttpFileWatcher(HttpServerConfig config, 
                std::unordered_map<std::string, std::string> * static_paths,
                Array<std::string> paths_to_dirs, pthread_mutex_t * mutex): 
                    FileWatcher(paths_to_dirs, IN_CLOSE | IN_CREATE | IN_MOVE | IN_DELETE), 
                    config(config), static_paths(static_paths), static_paths_mutex(mutex) {}

            void handle(const struct inotify_event *event);
    };
};
#endif