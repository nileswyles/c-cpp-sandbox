#include "web/client.h"

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "msc_unistd.h"

using namespace WylesLibs;

void Client::connect(const char * address, const uint16_t port) {
    int potential_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (potential_fd == -1) {
        // check errno for error
        loggerPrintf(LOGGER_DEBUG, "Error creating socket.\n");
    } else {
        struct in_addr addr; 
        if (inet_aton(address, &addr) == 0) {
            loggerPrintf(LOGGER_DEBUG, "Error parsing address string, %s\n", address);
        } else {
            struct sockaddr_in a = {
                AF_INET,
                ntohs(port),
                addr,
            };
            if (bind(potential_fd, (struct sockaddr *)(&a), sizeof(struct sockaddr_in)) == -1) {
                // check errno for error
                loggerPrintf(LOGGER_DEBUG, "error binding, %d\n", errno);
                return;
            } else {
                if (::connect(potential_fd) == -1) {
                    loggerPrintf(LOGGER_DEBUG, "Error connecting\n");
                } else {
                    this->fd = potential_fd;
                }
            }
        }
    }
}

void Client::disconnect() {
    if (this->fd != -1) {
        ::close(this->fd);
        this->fd = -1;
    }
}

void Client::write(const uint8_t * data, size_t size) {
    if (this->fd == -1) {
        throw std::runtime_error("Failed to send any data, the connnection is closed.");
    } else {
        ::write(this->fd, data, size);
    }
}

SharedArray<uint8_t> Client::read(size_t size) {
    SharedArray<uint8_t> data(size);
    if (this->fd == -1) {
        throw std::runtime_error("Failed to receive any data, the connnection is closed.");
    } else {
        ::read(this->fd, data.c_str(), size);
    }
    return data;
}