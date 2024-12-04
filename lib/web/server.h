#ifndef WYLESLIBS_SERVER_H
#define WYLESLIBS_SERVER_H

#include <stdint.h>

#include "threads/etasker.h"

#define SERVER_MINIMUM_CONNECTION_SPEED 131072 // 128kbps

namespace WylesLibs {
    typedef void(ServerConnectionTasker *)(ETasker * tasker, int fd, uint64_t initial_socket_timeout_s);

    class Server {
        protected:
            uint64_t initial_socket_timeout_s;
            ETasker tasker;
        public:
            Server() {
                initial_socket_timeout_s = 2;
                tasker = ETasker(SIZE_MAX, initial_socket_timeout_s);
            }
            virtual ~Server() = default;

            void disableConnectionTimeout(int fd);
            void setConnectionTimeout(int fd, uint32_t timeout_s);
            void setInitialConnectionTimeout(int fd, uint32_t timeout_s);
            void setSocketTimeout(int fd, uint32_t timeout_s);
            void setInitialSocketTimeout(int fd, uint32_t timeout_s);
            uint32_t getConnectionTimeout(int fd);
            uint32_t getSocketTimeout(int fd);
            void listen(const char * address, const uint16_t port);

            virtual void onConnection(int fd) = 0;
    };
};

#endif
