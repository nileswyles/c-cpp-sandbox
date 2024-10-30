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
class EStream {
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
    protected:
        void fillBuffer() override final;
    public:        
        SSL * ssl;
        SSLEStream(SSL * ssl): EStream(0) {
            ssl = ssl;
        }
        ~SSLEStream() override final = default;
        ssize_t writeBuffer(void * p_buf, size_t size) override final;
};
#endif

// lol
static constexpr std::string read_only_msg = "This EStream is locked for reading only";
class ReaderEStream: public EStream, public std::istream {
    private:
        ssize_t streamRead();
    protected:
        void cursorCheck() override final {}
        void fillBuffer() override final {}
    public:        
        std::shared_ptr<std::istream> reader;

        ReaderEStream(std::shared_ptr<std::istream> reader): EStream(0) {
            reader = reader;
        }
        virtual ~ReaderEStream() = default;
        uint8_t get() override final { return this->reader->get(); }
        uint8_t peek() override final { return this->reader->peek(); }
        SharedArray<uint8_t> readBytes(const size_t n) override final {
            return SharedArray<uint8_t>{this->reader, n};
        }
        size_t read() override final { return this->reader->read(); }
        size_t gcount() override final {
            return this->reader->gcount();
        }
        void seekg(size_t offset) override final {
            this->reader->seekg(offset);
        }
        bool good() {
            return this->reader->good();
        }
        bool eof() {
            return this->reader->eof();
        }
        // TODO: implement functionality as needed.

        // disabled functionality
        ssize_t writeBuffer(void * p_buf, size_t size) override final {
            throw std::runtime_error(read_only_msg);
        }
};

static constexpr std::string write_only_msg = "This EStream is locked for writing only";
class WriterEStream: public EStream, public std::ostream {
    private:
        ssize_t streamRead();
    protected:
        void cursorCheck() override final {
            throw std::runtime_error(write_only_msg);
        }
        void fillBuffer() override final {
            throw std::runtime_error(write_only_msg);
        }
    public:        
        std::shared_ptr<std::istream> writer;

        WriterEStream(std::shared_ptr<std::ostream> writer): EStream(0) {
            writer = writer;
        }
        virtual ~WriterEStream() = default;
        void put(char c) {
            this->writer->put(c);
        }
        void seekp(size_t offset) override final {
            this->writer->seekp(offset);
        }
        void write(const char * buffer, size_t size) override final {
            this->writer->write(buffer, size);
        }
        void flush() override final {
            this->writer->flush();
        }
        // disabled functionality
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
#ifdef WYLESLIBS_SSL_ENABLED
static_assert(sizeof(EStream) == 
    sizeof(uint8_t *) + 
    sizeof(size_t) + 
    sizeof(size_t) + 
    sizeof(size_t) +
    sizeof(SSL *) +
    sizeof(int) + 
    4 // just because?
);
static_assert(sizeof(EStream) == 48);
#else
static_assert(sizeof(EStream) == 
    sizeof(uint8_t *) + 
    sizeof(size_t) + 
    sizeof(size_t) + 
    sizeof(size_t) +
    sizeof(int) +
    4 // just because?
);
static_assert(sizeof(EStream) == 40);
#endif

}

#endif