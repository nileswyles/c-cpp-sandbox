#ifndef WYLESLIBS_CONNECTION_UPGRADER_H
#define WYLESLIBS_CONNECTION_UPGRADER_H

#include <string>

namespace WylesLibs::Http {
class ConnectionUpgrader {
    public:
        std::string path;
        std::string protocol;

        ConnectionUpgrader(std::string path, std::string protocol): path(path), protocol(protocol) {}

        virtual uint8_t onConnection(int conn_fd) = 0;
};

class WebsocketConnectionUpgrader: public ConnectionUpgrader {
    public:
        HttpConnectionUpgrader(std::string path, std::string protocol): ConnectionUpgrader(path, protocol) {}

        // this makes more sense extension of some Connection class?
        uint8_t onConnection(int conn_fd);
};

}

#endif