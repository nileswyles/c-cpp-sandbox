#ifndef WYLESLIBS_TRANSPORT_H
#define WYLESLIBS_TRANSPORT_H

#include <openssl/ssl.h>

#include <unistd.h>
#include <stdexcept>

class Transport {
    public:
        SSL * ssl;
        int conn_fd;
        Transport(int conn_fd) {
            if (conn_fd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            conn_fd = conn_fd;
            ssl = nullptr;
        }
        ssize_t readBuffer(void * buf, size_t size) {
            return read(this->conn_fd, buf, size);
        }
        ssize_t writeBuffer(void * buf, size_t size) {
            return write(this->conn_fd, buf, size);
        }
};

class SSLTransport: Transport {
    public:
        SSLTransport(SSL * ssl): SSLTransport(ssl, -1) {}
        SSLTransport(SSL * ssl, int conn_fd): Transport(conn_fd) {
            ssl = ssl;
            conn_fd = conn_fd;
        }
        ssize_t readBuffer(void * buf, size_t size) {
            return SSL_read(this->ssl, buf, size);
        }
        ssize_t writeBuffer(void * buf, size_t size) {
            return SSL_write(this->ssl, buf, size);
        }
};
#endif