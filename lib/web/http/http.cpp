#include "http.h"

#include "web/http/http_connection_etask.h"

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_HTTP
#define LOGGER_LEVEL_HTTP GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_HTTP
#define LOGGER_HTTP 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_HTTP

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_HTTP
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Http;

int HttpServer::onConnection(int fd) {
    // thread abstraction, this launches a thread and processes http request...
    // ESharedPtr<HttpConnectionETask> task(new HttpConnectionETask(fd, this->initial_socket_timeout_s, this));
    ESharedPtr<HttpConnectionETask> task(new HttpConnectionETask(fd, this->initial_socket_timeout_s));
    return this->tasker.run(task);
}