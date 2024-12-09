#ifndef WYLESLIBS_SERVER_CONNECTION_ETASK_H
#define WYLESLIBS_SERVER_CONNECTION_ETASK_H

#include "threads/etasker.h"

namespace WylesLibs {
    class ServerConnectionETask: public ETask {
        protected:
            int fd;
            uint64_t socket_timeout_s;
        public:
            ServerConnectionETask(int fd, uint64_t socket_timeout_s): fd(fd), socket_timeout_s(socket_timeout_s) {}
            virtual ~ServerConnectionETask() = default;
            void initialize() override;
    };
}
#endif