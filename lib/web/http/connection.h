#ifndef WYLESLIBS_CONNECTION_UPGRADER_H
#define WYLESLIBS_CONNECTION_UPGRADER_H

#include <string>

namespace WylesLibs::Http {

class Connection {
    public:
        Connection() {}

        virtual uint8_t onConnection(int conn_fd) = 0;
};

class ConnectionUpgrader: public Connection {
    public:
        std::string path;
        std::string protocol;

        ConnectionUpgrader(std::string path, std::string protocol): path(path), protocol(protocol) {}

        virtual uint8_t onConnection(int conn_fd) = 0;
};

}

#endif