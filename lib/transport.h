#ifndef WYLESLIBS_TRANSPORT_IO_H
#define WYLESLIBS_TRANSPORT_IO_H

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
        int32_t ioRead(void * buf, size_t size) {
            return read(this->conn_fd, buf, size);
        }
        int32_t ioWrite(void * buf, size_t size) {
            return write(this->conn_fd, buf, size);
        }
};

class SSLTransport: Transport {
    public:
        SSLTransport(SSL * ssl): Transport(conn_fd) {
            ssl = ssl;
        }
        int32_t ioRead(void * buf, size_t size) {
            return SSL_read(this->ssl, buf, size);
        }
        int32_t ioWrite(void * buf, size_t size) {
            return SSL_write(this->ssl, buf, size);
        }
};
#endif