#include "web/http/http_file_watcher.h"

using namespace WylesLibs::Http;

void HttpFileWatcher::handle(const struct inotify_event *event) {
    loggerPrintf(LOGGER_DEBUG, "EVENT MASK: %d", event->mask);
    if (!(event->mask & IN_ISDIR)) { // files only
        // TODO:
        // CODE SMELL?
        pthread_mutex_lock(static_paths_mutex);
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
        pthread_mutex_unlock(static_paths_mutex);
    }
}