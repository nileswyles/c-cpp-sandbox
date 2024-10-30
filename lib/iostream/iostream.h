#ifndef WYLESLIBS_IOSTREAM_H
#define WYLESLIBS_IOSTREAM_H

#include "reader_task.h"

#include "datastructures/array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef WYLESLIBS_SSL_ENABLED
#include <openssl/ssl.h>
#endif 

#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {
class IOStream: public std::istream {
    private:
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        
        void fillBuffer();
        void cursorCheck();
    public:        
#ifdef WYLESLIBS_SSL_ENABLED
        SSL * ssl;
#endif
        int fd;
        bool managed_fd;
        IOStream() {}
        IOStream(std::string file_path) {
            int fd = open(file_path.c_str(), O_RDONLY);
            if (fd == -1) {
                throw std::runtime_error("Unable to read file at: " + file_path);
            }
            managed_fd = true;
        }
        IOStream(uint8_t * p_buf, const size_t p_buf_size) {
            buf = p_buf;
            buf_size = p_buf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown if read past buffer. (see fillBuffer implementation)
            fd = -1;
            bytes_in_buffer = p_buf_size;
            managed_fd = false;
        }
#ifdef WYLESLIBS_SSL_ENABLED
        IOStream(SSL * ssl): IOStream(0) {
            ssl = ssl;
        }
#endif
        IOStream(const int fd): IOStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        IOStream(const int p_fd, const size_t p_buf_size) {
            if (p_fd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (p_buf_size < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            buf_size = p_buf_size;
            cursor = 0;
            fd = p_fd;
            bytes_in_buffer = 0;
            buf = newCArray<uint8_t>(buf_size);
#ifdef WYLESLIBS_SSL_ENABLED
            ssl = nullptr;
#endif
            managed_fd = false;
        }
        ~IOStream() {
            delete[] buf;
            if (true == managed_fd) {
                close(fd);
            }
        };
        ssize_t writeBuffer(void * p_buf, size_t size);
        uint8_t peekByte();
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        uint8_t readByte();
        SharedArray<uint8_t> readBytes(const size_t n);
        // ! IMPORTANT - inclusive means we read and consume the until character. 
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        SharedArray<uint8_t> readUntil(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true);

        void readDecimal(double& value, size_t& digit_count);
        void readNatural(double& value, size_t& digit_count);
};

// @ static

// assuming amd64 - what year are we in? LMAO
#ifdef WYLESLIBS_SSL_ENABLED
static_assert(sizeof(IOStream) == 
    sizeof(uint8_t *) + 
    sizeof(size_t) + 
    sizeof(size_t) + 
    sizeof(size_t) +
    sizeof(SSL *) +
    sizeof(int) + 
    4 // just because?
);
static_assert(sizeof(IOStream) == 48);
#else
static_assert(sizeof(IOStream) == 
    sizeof(uint8_t *) + 
    sizeof(size_t) + 
    sizeof(size_t) + 
    sizeof(size_t) +
    sizeof(int) +
    4 // just because?
);
static_assert(sizeof(IOStream) == 40);
#endif

}

#endif