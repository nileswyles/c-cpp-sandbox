#ifndef WYLESLIBS_IOSTREAM_H
#define WYLESLIBS_IOSTREAM_H

#include "reader_task.h"

#include "datastructures/array.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>
#include <memory>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#ifdef WYLESLIBS_SSL_ENABLED
#include <openssl/ssl.h>
#endif 

// TODO: think about this value again... 8k * 1Million is what? 8G seems like alot...?
#define READER_RECOMMENDED_BUF_SIZE 8096

namespace WylesLibs {
// TODO:
//      consider using stream instead of buf for reads and extend stream to support stream api in addition to readUntil stuff.
//      support range requests for file_gcs because apparently it currently downloads the entire file.
class EStream {
    /*
        Read and Write from file descriptor
    */
    protected:
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;
        
        virtual void fillBuffer();
        virtual void cursorCheck();
    public: 
        int fd;
        EStream() {}
        EStream(uint8_t * p_buf, const size_t p_buf_size) {
            // # testing.
            buf = p_buf;
            buf_size = p_buf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown if read past buffer. (see fillBuffer implementation)
            fd = -1;
            bytes_in_buffer = p_buf_size;
        }
        EStream(const int fd): EStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        EStream(const int p_fd, const size_t p_buf_size) {
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
        }
        virtual ~EStream() = default;
        virtual ssize_t writeBuffer(void * p_buf, size_t size);
        virtual uint8_t peek();
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        virtual uint8_t get();
        virtual SharedArray<uint8_t> readBytes(const size_t n);
        // ! IMPORTANT - inclusive means we read and consume the until character. 
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        virtual SharedArray<uint8_t> readUntil(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true);

        virtual void readDecimal(double& value, size_t& digit_count);
        virtual void readNatural(double& value, size_t& digit_count);
};

#ifdef WYLESLIBS_SSL_ENABLED
class SSLEStream: public EStream {
    /*
        Read and Write from openssl object
    */
    protected:
        void fillBuffer() override final;
    public:        
        SSL * ssl;
        SSLEStream(SSL * ssl): EStream(0) {
            ssl = ssl;
        }
        ~SSLEStream() override final {
            // Move acceptTLS here?
            SSL_shutdown(this->ssl);
            SSL_free(this->ssl);
        };
        ssize_t writeBuffer(void * p_buf, size_t size) override final;
};
#endif

// TODO: timeouts but for files should be fine? And I trust google, I think...
static const std::string read_only_msg("This EStream is locked for reading only");
class ReaderEStream: public EStream, public std::basic_istream<char> {
    /*
        Read from stream
    */
    protected:
        void cursorCheck() override final {}
        void fillBuffer() override final {}
    public:        
        std::shared_ptr<std::basic_istream<char>> reader;

        // TODO: std::move? that's interesting
        ReaderEStream(std::shared_ptr<std::basic_istream<char>> reader): EStream(0), std::basic_istream<char>(std::move(*reader)) {
            reader = reader;
        }
        virtual ~ReaderEStream() = default;
        // also, how does it handle EStream::get vs istream::get? Do I really need to explicitly override that.
        uint8_t get() override final { return this->reader->get(); }
        uint8_t peek() override final { return this->reader->peek(); }
        SharedArray<uint8_t> readBytes(const size_t n) override final {
            // yuck
            // TODO: casting is annoying...
            return SharedArray<uint8_t>(std::reinterpret_pointer_cast<std::basic_istream<uint8_t>>(this->reader), n);
        }
        // disabled functionality from EStream
        ssize_t writeBuffer(void * p_buf, size_t size) override final {
            throw std::runtime_error(read_only_msg);
        }
};

static const std::string write_only_msg("This EStream is locked for writing only");
class WriterEStream: public EStream, public std::ostream {
    /*
        Write to stream
    */
    protected:
        void cursorCheck() override final {
            throw std::runtime_error(write_only_msg);
        }
        void fillBuffer() override final {
            throw std::runtime_error(write_only_msg);
        }
    public:        
        std::shared_ptr<std::basic_istream<char>> writer;

        WriterEStream(std::shared_ptr<std::ostream> writer): EStream(0), std::ostream(std::move(*writer)) {
            writer = writer;
        }
        virtual ~WriterEStream() = default;
        // disabled functionality from EStream
        uint8_t get() override final { 
            throw std::runtime_error(write_only_msg);
        }
        uint8_t peek() override final { 
            throw std::runtime_error(write_only_msg);
        }
        SharedArray<uint8_t> readBytes(const size_t n) override final {
            throw std::runtime_error(write_only_msg);
        }
        SharedArray<uint8_t> readUntil(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true) override final {
            throw std::runtime_error(write_only_msg);
        }
        void readDecimal(double& value, size_t& digit_count) {
            throw std::runtime_error(write_only_msg);
        }
        void readNatural(double& value, size_t& digit_count) {
            throw std::runtime_error(write_only_msg);
        }
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

}

#endif