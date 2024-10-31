#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "estream/estream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <sys/stat.h>

#include <fstream>
#include <memory>
#include <set>

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {

static void write(std::shared_ptr<std::basic_ostream<char>> s, SharedArray<uint8_t> buffer, bool append = false) {
    if (append) {
        s->seekp(std::ios_base::end);
    } else {
        // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
        s->seekp(0);
    }
    s->write((const char *)buffer.start(), buffer.size()); // binary output
    s->flush();
}

static void write(std::shared_ptr<std::basic_ostream<char>> s, SharedArray<uint8_t> buffer, size_t offset = 0) {
    s->seekp(offset);
    s->write((const char *)buffer.start(), buffer.size()); // binary output
    s->flush();
}

static SharedArray<uint8_t> read(std::shared_ptr<ReaderEStream> s, size_t offset = 0, size_t size = SIZE_MAX) {
    SharedArray<uint8_t> file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (s->good() && !s->eof()) {
            file_data.append(s->get());
        }
        if (false == s->good()) {
            throw std::runtime_error("Error occured while reading istream until EOF.");
        }
    } else {
        file_data = s->readBytes(size);
    }
    return file_data;
}

class FileManager {
    protected:
        std::shared_ptr<FileManager> this_shared;
        std::set<std::string> writers;
        pthread_mutex_t writers_lock;
    public:
        FileManager() {
            pthread_mutex_init(&writers_lock, nullptr);
        };
        virtual ~FileManager() = default;

        void write(std::string path, SharedArray<uint8_t> buffer, bool append) {
            // annoying but necessary
            File::write(this->writer(path), buffer, append);

            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->removeWriter(path);
        }
        void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0) {
            File::write(this->writer(path), buffer, offset);
            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->removeWriter(path);
        }
        SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
            return File::read(this->reader(path), offset, size);
        }

        virtual std::shared_ptr<ReaderEStream> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX);
        virtual std::shared_ptr<std::basic_ostream<char>> writer(std::string path);
        // ! IMPORTANT - implementation should call this function when done with writer...
        //      is there a better way? yeah, maybe different ostream type with shared_ptr to this stuff... that removes when close is called...? too complicated?
        virtual void removeWriter(std::string path);

        virtual uint64_t stat(std::string path);
        virtual SharedArray<std::string> list(std::string path);

        virtual void remove(std::string path);
        virtual void move(std::string path, std::string destination_path);
        virtual void copy(std::string path, std::string destination_path);
};
}
#endif