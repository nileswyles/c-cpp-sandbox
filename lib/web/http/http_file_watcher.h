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
            
            pthread_mutex_t * mutex;

            HttpFileWatcher(HttpServerConfig config, 
                std::unordered_map<std::string, std::string> * static_paths,
                Array<std::string> paths_to_dirs, pthread_mutex_t * mutex): 
                    FileWatcher(paths_to_dirs, IN_CLOSE | IN_CREATE | IN_MOVE | IN_DELETE), 
                    config(config), static_paths(static_paths), mutex(mutex) {}

            void handle(const struct inotify_event *event) {
                loggerPrintf(LOGGER_DEBUG, "EVENT MASK: %d", event->mask);
                if (!(event->mask & IN_ISDIR)) { // files only

                    // TODO:
                    // CODE SMELL?
                    pthread_mutex_lock(mutex);
                    if (event->mask & IN_DELETE) {
                        static_paths->erase(event->name);
                    } else {
                        if (!static_paths->contains(event->name)) {
                            // new file detected!
                            loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
                            std::string path = "";
                            std::string ext = "";
							if (ext == ".html") {
								(*static_paths)[path] = "text/html";
							} else if (ext == ".js") {
								(*static_paths)[path] = "text/javascript";
							} else if (ext == ".css") {
								(*static_paths)[path] = "text/css";
                            } else {
								(*static_paths)[path] = "none";
                            }
                            loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
                        }
                    }
                    pthread_mutex_unlock(mutex);
                }
            }
    };
};
#endif