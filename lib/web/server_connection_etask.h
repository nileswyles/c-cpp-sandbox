#ifndef WYLESLIBS_SERVER_CONNECTION_ETASK_H
#define WYLESLIBS_SERVER_CONNECTION_ETASK_H

#include "threads/etasker.h"

namespace WylesLibs {
    class ServerConnectionETask: public ETask {
        protected:
            int fd;
            uint64_t socket_timeout;
        public:
            ServerETask(int fd, uint64_t socket_timeout): fd(fd), socket_timeout(socket_timeout) {}
            ~ServerETask() = default;
            void initialize() override;
    };
}
#endif