#include "http_file_watcher.h"
#include "paths.h"

using namespace WylesLibs::Http;
using namespace WylesLibs::Paths;

void HttpFileWatcher::handle(const struct inotify_event * event) {
    loggerPrintf(LOGGER_DEBUG, "EVENT MASK: %d", event->mask);
    if (!(event->mask & IN_ISDIR)) { // files only
        pthread_mutex_lock(this->static_paths.getMutex());
        if (event->mask & IN_DELETE) {
            this->static_paths.erase(event->name);
        } else {
            if (!this->static_paths.contains(event->name)) {
                loggerPrintf(LOGGER_DEBUG, "Static Paths: %s\n", config.static_path.c_str());
                loggerPrintf(LOGGER_DEBUG, "File Name: %s\n", event->name);
                this->static_paths[event->name] = contentTypeFromPath(event->name);
            }
        }
        pthread_mutex_unlock(this->static_paths.getMutex());
    }
}