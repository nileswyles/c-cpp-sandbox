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

static void write(std::shared_ptr<std::ostream> s, SharedArray<uint8_t> buffer, bool append = false) {
    if (append) {
        s->seekp(std::ios_base::end);
    } else {
        // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
        s->seekp(0);
    }
    s->write((const char *)buffer.start(), buffer.size()); // binary output
    s->flush();
}

static void write(std::shared_ptr<std::ostream> s, SharedArray<uint8_t> buffer, size_t offset = 0) {
    s->seekp(offset);
    s->write((const char *)buffer.start(), buffer.size()); // binary output
    s->flush();
}

static SharedArray<uint8_t> read(std::shared_ptr<std::istream> s, size_t offset = 0, size_t size = SIZE_MAX) {
    SharedArray<uint8_t> file_data;
    if (offset == 0 && size == SIZE_MAX) {
        // read until EOF
        while (s->good() && !s->eof()) {
            file_data.append(s->get());
        }
        if (false == s->good()) {
            throw std::runtime_error("Error occured while reading istream until EOF.");
        }
    } else {
        s->seekg(offset);
        file_data = SharedArray<uint8_t>(*s, size);
    }
    return file_data;
}

class FileManager {
    protected:
        std::set<std::string> writers;
        pthread_mutex_t writers_lock;
    public:
        FileManager() = default;
        virtual ~FileManager() = default;

        void write(std::string path, SharedArray<uint8_t> buffer, bool append) {
            // annoying but necessary
            std::shared_ptr<std::ostream> s = std::dynamic_pointer_cast<std::ostream>(this->writer(path));
            File::write(s, buffer, append);

            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->removeWriter(path);
        }
        void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0) {
            std::shared_ptr<std::ostream> s = std::dynamic_pointer_cast<std::ostream>(this->writer(path));
            File::write(s, buffer, offset);
            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            this->removeWriter(path);
        }
        SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
            return File::read(std::dynamic_pointer_cast<std::istream>(this->reader(path)), offset, size);
        }

        virtual std::shared_ptr<ReaderEStream> reader(std::string path);
        virtual std::shared_ptr<WriterEStream> writer(std::string path);
        // ! IMPORTANT - implementation should call this function when done with writer...
        //      is there a better way? yeah, maybe different ostream type with shared_ptr to this stuff... that removes when close is called...? too complicated?
        virtual void removeWriter(std::string path);

        virtual struct stat stat(std::string path);
        virtual SharedArray<std::string> list(std::string path);

        virtual void remove(std::string path);
        virtual void move(std::string path, std::string destination_path);
        virtual void copy(std::string path, std::string destination_path);
};
}
#endif