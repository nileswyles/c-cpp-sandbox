#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "estream/estream.h"
#include "file/stream_factory.h"

#include <ios>
#include <unistd.h>
#include <string>

#include <memory>

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {

static const std::string stream_error_msg("The stream provided has an error.");

static void write(std::shared_ptr<std::basic_ostream<char>> s, SharedArray<uint8_t> buffer, bool append = false) {
    if (true == append) {
        s->seekp(0, std::basic_ostream<char>::end);
    } else {
        // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
        s->seekp(0);
    }
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Writing to file stream:\n%s\n", buffer.toString().c_str());
    s->write((char *)buffer.begin(), buffer.size()); // binary output
    s->flush();
}

static void write(std::shared_ptr<std::basic_ostream<char>> s, SharedArray<uint8_t> buffer, size_t offset = 0) {
    s->seekp(offset);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Writing to file stream:\n%s\n", buffer.toString().c_str());
    s->write((const char *)buffer.begin(), buffer.size()); // binary output
    s->flush();
}

static SharedArray<uint8_t> read(std::shared_ptr<ReaderEStream> s, size_t offset = 0, size_t size = SIZE_MAX) {
    SharedArray<uint8_t> file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (true == s->good()) { //  && false == s->eof(); implied
            uint8_t c = s->get();
            // good and eof are true, false (respectively) at eof character, so ignore.
            if (c != 0xFF) {
                file_data.append(c);
            }
        }
        // if (true == s->fail()) {
        //     throw std::runtime_error("Error occured while reading istream until EOF.");
        // }
    } else {
        file_data = s->readBytes(size);
    }
    return file_data;
}

class FileManager {
    protected:
        std::shared_ptr<StreamFactory> stream_factory;
    public:
        FileManager(): stream_factory(std::make_shared<StreamFactory>()) {}
        FileManager(std::shared_ptr<StreamFactory> stream_factory): stream_factory(stream_factory) {}
        virtual ~FileManager() = default;

        void write(std::string path, SharedArray<uint8_t> buffer, bool append = false) {
            File::write(this->stream_factory->writer(path), buffer, append);

            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->streams()->removeWriter(path);
        }
        void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0) {
            File::write(this->stream_factory->writer(path), buffer, offset);
            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->streams()->removeWriter(path);
        }
        SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
            std::shared_ptr<ReaderEStream> s = std::make_shared<ReaderEStream>(this->stream_factory, path, offset, size);
            return File::read(s, offset, size);
        }
        std::shared_ptr<StreamFactory> streams() {
            return this->stream_factory;
        }

        virtual uint64_t stat(std::string path);
        virtual SharedArray<std::string> list(std::string path);

        virtual void remove(std::string path);
        virtual void move(std::string path, std::string destination_path);
        virtual void copy(std::string path, std::string destination_path);
};
}
#endif