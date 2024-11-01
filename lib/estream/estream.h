#ifndef WYLESLIBS_ESTREAM_H
#define WYLESLIBS_ESTREAM_H

#include "estream/reader_task.h"

#include "file/stream_factory.h"
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

using namespace WylesLibs::File;

namespace WylesLibs {
class ReaderEStream {
    /*
        Read from stream
    */
    // TODO: does this incur any additional overhead in inherited even though private?
    private:
        std::shared_ptr<std::basic_istream<char>> reader;
        std::shared_ptr<StreamFactory> factory;
        std::string path;
        size_t file_offset;
        size_t chunk_size;
    protected:
        virtual void cursorCheck();
        virtual void fillBuffer();
    public: 
        ReaderEStream() = default;
        // TODO: std::move? that's interesting
        ReaderEStream(std::shared_ptr<std::basic_istream<char>> reader) {
            factory = nullptr;
            reader = reader;
        }
        ReaderEStream(std::shared_ptr<StreamFactory> factory, std::string path, size_t initial_offset = 0, size_t chunk_size = SIZE_MAX) {
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
        void seekg(size_t offset);

        virtual SharedArray<uint8_t> readBytes(const size_t n);
        // ! IMPORTANT - inclusive means we read and consume the until character. 
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        virtual SharedArray<uint8_t> readUntil(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true);

        virtual void readDecimal(double& value, size_t& digit_count);
        virtual void readNatural(double& value, size_t& digit_count);
};

class EStream: public ReaderEStream {
    /*
        Read and Write from file descriptor
    */
    protected:
        uint8_t * buf;
        size_t buf_size;
        size_t cursor;
        size_t bytes_in_buffer;

        void cursorCheck() override final;
        void fillBuffer() override;
    public: 
        int fd;
        EStream() = default;
        EStream(uint8_t * p_buf, const size_t p_buf_size): ReaderEStream() {
            // # testing.
            buf = p_buf;
            buf_size = p_buf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown if read past buffer. (see fillBuffer implementation)
            fd = -1;
            bytes_in_buffer = p_buf_size;
        }
        EStream(const int fd): EStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        EStream(const int p_fd, const size_t p_buf_size): ReaderEStream() {
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
        // standard istream
        uint8_t get() override;
        uint8_t peek() override;
        bool eof() override;
        bool good() override;
        // char_type read() override;

        // ReaderEstream
        SharedArray<uint8_t> readBytes(const size_t n) override;
        // Write to FD
        virtual ssize_t write(void * p_buf, size_t size);
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
        SSLEStream() = default;
        SSLEStream(SSL * ssl): EStream(0) {
            ssl = ssl;
        }
        ~SSLEStream() override final {
            // Move acceptTLS here?
            SSL_shutdown(this->ssl);
            SSL_free(this->ssl);
        };
        ssize_t write(void * p_buf, size_t size) override final;
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