#ifndef WYLESLIBS_HTTP_FILE_WATCHER_H
#define WYLESLIBS_HTTP_FILE_WATCHER_H

#include "file_watcher.h"
#include "thread_safe_map.h"
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
            ThreadSafeMap<std::string, std::string> static_paths;
            HttpFileWatcher(HttpServerConfig config, ThreadSafeMap<std::string, std::string> static_paths, Array<std::string> paths_to_dirs): 
                    FileWatcher(paths_to_dirs, IN_CLOSE | IN_CREATE | IN_MOVE | IN_DELETE), config(config), static_paths(static_paths) {}
            ~HttpFileWatcher() override = default;

            void handle(const struct inotify_event *event);
    };
};
#endif