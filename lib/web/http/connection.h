#ifndef WYLESLIBS_CONNECTION_UPGRADER_H
#define WYLESLIBS_CONNECTION_UPGRADER_H

#include <string>

namespace WylesLibs::Http {

class Connection {
    public:
        Connection() {}
        virtual ~Connection() = default;
        virtual uint8_t onConnection(int conn_fd) = 0;
};

class ConnectionUpgrader {
    public:
        std::string path;
        std::string protocol;

        ConnectionUpgrader(std::string path, std::string protocol): path(path), protocol(protocol) {}
        virtual ~ConnectionUpgrader() = default;

        virtual uint8_t onConnection(IOStream * io) = 0;
};

}

#endif