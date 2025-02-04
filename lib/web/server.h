#ifndef WYLESLIBS_SERVER_H
#define WYLESLIBS_SERVER_H

#include <stdint.h>

#include "key_generator.h"
#include "threads/etasker.h"
#include "file/file.h"
#include "web/server_config.h"

#define SERVER_MINIMUM_CONNECTION_SPEED 131072 // 128kbps
#define INITIAL_CONNECTION_TIMEOUT_S 15

namespace WylesLibs {
    class Server {
        protected:
            uint64_t initial_socket_timeout_s;
            ETasker tasker;
        public:
            ServerConfig config;
            ESharedPtr<FileManager> file_manager;
            UniqueKeyGenerator key_generator;

            Server() {
                initial_socket_timeout_s = 2;
                tasker = ETasker(SIZE_MAX, INITIAL_CONNECTION_TIMEOUT_S);
            }
            virtual ~Server() = default;

            void disableConnectionTimeout(int fd);
            void setConnectionTimeout(int fd, uint64_t timeout_s);
            void setInitialConnectionTimeout(int fd, uint64_t timeout_s);
            void setSocketTimeout(int fd, uint64_t timeout_s);
            void setInitialSocketTimeout(int fd, uint64_t timeout_s);
            uint64_t getConnectionTimeout(int fd);
            uint64_t getSocketTimeout(int fd);
            void listen(const char * address, const uint16_t port);

            virtual int onConnection(int fd) = 0;
    };

    extern Server * getServerContext();
};

#endif
