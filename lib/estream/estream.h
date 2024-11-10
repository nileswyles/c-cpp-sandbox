#ifndef WYLESLIBS_ESTREAM_H
#define WYLESLIBS_ESTREAM_H

#include "estream/reader_task.h"

#include "file/stream_factory.h"
#include "datastructures/array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>
#include <memory>
#include <ios>
#include <istream>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <poll.h>

#ifdef WYLESLIBS_SSL_ENABLED
#include <openssl/ssl.h>
#endif

#define READER_RECOMMENDED_BUF_SIZE 8096

// ! IMPORTANT - 
//      server.c::MAX_CONNECTIONS is (1 << 16) 64KB... 
//       64k * 4048 == 249072KB or ~256MB
#define READER_RECOMMENDED_BUF_SIZE_SSL 4048

namespace WylesLibs {
class ReaderEStream {
    /*
        Read from stream
    */
    // TODO: does this incur any additional overhead in inherited even though private?
    private:
        std::shared_ptr<std::basic_istream<char>> reader;
        std::shared_ptr<File::StreamFactory> factory;
        std::string path;
        size_t file_offset;
        size_t chunk_size;
    protected:
        virtual bool readPastBuffer();
        virtual void fillBuffer();
    public:
        ReaderEStream() = default;
        // TODO: std::move? that's interesting
        ReaderEStream(std::shared_ptr<std::basic_istream<char>> reader) {
            factory = nullptr;
            reader = reader;
        }
        ReaderEStream(std::shared_ptr<File::StreamFactory> factory, std::string path, size_t initial_offset = 0, size_t chunk_size = SIZE_MAX) {
            factory = factory;
            path = path;
            file_offset = initial_offset;
            chunk_size = chunk_size;
            reader = factory->reader(path, initial_offset, chunk_size);
        }
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        virtual ~ReaderEStream() = default;
        // standard istream
        virtual uint8_t get();
        virtual uint8_t peek();
        virtual bool eof();
        virtual bool good();
        virtual bool fail();
        void seekg(size_t offset);

        virtual SharedArray<uint8_t> readBytes(const size_t n);
        // ! IMPORTANT - inclusive means we read and consume the until character.
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        virtual SharedArray<uint8_t> readUntil(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true);

        virtual void readDecimal(double &value, size_t &digit_count);
        virtual void readNatural(double &value, size_t &digit_count);
};

class EStream: public ReaderEStream {
    /*
        Read and Write from file descriptor
    */
    private:
        struct pollfd poll_fd;
        std::ios_base::iostate flags;
    protected:
        uint8_t *buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        bool readPastBuffer() override final;
        void fillBuffer() override;
    public:
        int fd;
        EStream() = default;
        EStream(uint8_t * p_buf, const size_t p_buf_size): ReaderEStream() {
            flags = std::ios_base::goodbit;
            buf = p_buf;
            buf_size = p_buf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown and flags are updated if read past buffer. (see fillBuffer implementation)
            fd = -1;
            bytes_in_buffer = p_buf_size;
        }
        EStream(const int fd): EStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        EStream(const int p_fd, const size_t p_buf_size): ReaderEStream() {
            flags = std::ios_base::goodbit;
            if (p_fd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (p_buf_size < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            poll_fd.fd = p_fd;
            poll_fd.events = POLLIN;
            buf_size = p_buf_size;
            cursor = 0;
            fd = p_fd;
            bytes_in_buffer = 0;
            buf = newCArray<uint8_t>(buf_size);
        }
        ~EStream() override = default;
        // standard istream
        uint8_t get() override;
        uint8_t peek() override;
        bool eof() override;
        bool good() override;
        bool fail() override;
        // char_type read() override;

        // ReaderEstream
        SharedArray<uint8_t> readBytes(const size_t n) override;
        // Write to FD
        virtual ssize_t write(void *p_buf, size_t size);
};

#ifdef WYLESLIBS_SSL_ENABLED
class SSLEStream: public EStream {
    /*
        Read and Write from openssl object
    */
    private:
        SSL * ssl;
        static SSL * acceptTLS(SSL_CTX * context, int fd, bool client_auth_enabled);
    protected:
        void fillBuffer() override final;
    public:
        SSLEStream() = default;
        SSLEStream(SSL_CTX * context, int fd, bool client_auth_enabled): EStream(fd, READER_RECOMMENDED_BUF_SIZE_SSL) {
            ssl = acceptTLS(context, fd, client_auth_enabled);
        }
        ~SSLEStream() override final {
            if (this->ssl != nullptr) {
                SSL_shutdown(this->ssl);
                SSL_free(this->ssl);
            }
        }
        ssize_t write(void *p_buf, size_t size) override final;
};
#endif
};

// @ static

// assuming amd64 - what year are we in? LMAO
// static_assert(sizeof(EStream) ==
//     sizeof(char *) +
//     sizeof(size_t) +
//     sizeof(size_t) +
//     sizeof(size_t) +
//     sizeof(int) +
//     4 // just because?
// );
// static_assert(sizeof(EStream) == 32);

#endif