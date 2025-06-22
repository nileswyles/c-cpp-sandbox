#ifndef WYLESLIBS_CLIENT_H
#define WYLESLIBS_CLIENT_H

#include "stdint.h"
#include "datastructures/array.h"

namespace WylesLibs {
    class Client {
        public:
            int fd;
            std::string address;
            Client() {
                fd = -1;
            }
            Client(std::string address): address(address), Client() {}
            virtual ~Client() = default;

            void connect(const char * address, const uint16_t port);
            void disconnect();
            void write(const uint8_t * data, size_t size);
            SharedArray<uint8_t> read(size_t size = 1024);

            bool operator==(const Client& other) {
                return this->address == other.address;
            }
            bool operator!=(const Client& other) {
                return !(*this == other);
            }
    };
};

#endif