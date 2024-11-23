#ifndef WYLESLIBS_ISTREAMESTREAM_H
#define WYLESLIBS_ISTREAMESTREAM_H

#include "estream/byteestream.h"
#include "file/stream_factory.h"

#include <string>
#include <stdexcept>
#include <memory>
#include <ios>
#include <istream>

namespace WylesLibs {
class IStreamEStream: public ByteEStream {
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
        IStreamEStream() = default;
        // TODO: std::move? that's interesting
        IStreamEStream(std::shared_ptr<std::basic_istream<char>> reader) {
            factory = nullptr;
            reader = reader;
        }
        IStreamEStream(std::shared_ptr<File::StreamFactory> factory, std::string path, size_t initial_offset = 0, size_t chunk_size = SIZE_MAX) {
            factory = factory;
            path = path;
            file_offset = initial_offset;
            chunk_size = chunk_size;
            reader = factory->reader(path, initial_offset, chunk_size);
        }
        // peek until doesn't make much sense with static sized buffer... so let's omit for now...
        // peek bytes cannot exceed bytes_left_in_buffer? so let's also omit...
        virtual ~IStreamEStream() = default;

        // TODO:
        // review these again...

        // standard istream
        uint8_t get() override;
        uint8_t peek() override;
        void unget() override;
        bool eof() override;
        bool good() override;
        bool fail() override;
        virtual void seekg(size_t offset);

        SharedArray<uint8_t> read(const size_t n, StreamTask<uint8_t, SharedArray<uint8_t>> * operation = nullptr) override;

        ssize_t write(uint8_t * p_buf, size_t size) override;

        IStreamEStream(IStreamEStream && x) = default;
        IStreamEStream& operator=(IStreamEStream && x) = default;

        bool operator!() {
            return this->good();
        }
        // bool operator bool() {
        //     return this->good();
        // }
};

};

#endif