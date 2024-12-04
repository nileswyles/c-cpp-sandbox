#ifndef WYLESLIBS_HTTP_CONNECTION_ETASK_H
#define WYLESLIBS_HTTP_CONNECTION_ETASK_H

#include "web/server_connection_etask.h"

namespace WylesLibs {
    class HttpConnectionETask: public ServerConnectionTask {
        private:
            int fd;
            uint64_t socket_timeout;
            HttpServer * connection;
        public:
            HttpConnectionETask(int fd, uint64_t socket_timeout, HttpServer * connection): ServerConnectionETask(fd, socket_timeout), connection(connection) {}
            ~HttpConnectionETask() {}
            void run() override;
            void onExit() override;
    };
}
#endif