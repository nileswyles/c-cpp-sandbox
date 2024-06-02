#ifndef WYLESLIBS_CONNECTION_UPGRADER_H
#define WYLESLIBS_CONNECTION_UPGRADER_H

#include <string>

namespace WylesLibs::Http {

class Connection {
    public:
        // lol.... yeah this might not be necessary but ... compiler errors/static checking a good practice? lol
        Connection() {}

        virtual uint8_t onConnection(int conn_fd) = 0;
};

// lol... yeah this might be too much...
class ConnectionUpgrader: public Connection {
    public:
        std::string path;
        std::string protocol;

        // lol.... yeah this might not be necessary but ... compiler errors/static checking a good practice? lol
        ConnectionUpgrader(std::string path, std::string protocol): path(path), protocol(protocol) {}

        // this required right or compiler error?
        virtual uint8_t onConnection(int conn_fd) = 0;
};

}

#endif