#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "iostream/iostream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <sys/stat.h>

#include <fstream>
#include <memory>

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {

static void write(std::string file_path, std::string buffer, bool append) {
    // open every time a problem?
    std::fstream s{file_path, s.binary | s.out};
    if (!s.is_open()) {
        throw std::runtime_error("Unable to open file at: " + file_path);
    } else {
        if (append) {
            s.seekp(0, std::ios_base::end);
        } else {
            s.seekp(0);
        }
        s.write((const char *)buffer.data(), buffer.size()); // binary output
        s.flush();
        s.close();
    }
}

static void write(std::string path, SharedArray<uint8_t> buffer, bool append) {
    // open every time a problem?
    std::fstream s{path, s.binary | s.out};
    if (!s.is_open()) {
        throw std::runtime_error("Unable to open file at: " + path);
    } else {
        if (append) {
            s.seekp(0, std::ios_base::end);
        } else {
            // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
            s.seekp(0);
        }
        s.write((const char *)buffer.start(), buffer.size()); // binary output
        s.flush();
        s.close();
    }
}

static WylesLibs::SharedArray<uint8_t> read(std::string file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Unable to read file at: " + file_path);
    }
    struct stat stat_info = {};
    int lol = fstat(fd, &stat_info);
    IOStream r(fd);
    SharedArray<uint8_t> file = r.readBytes(stat_info.st_size);
    close(fd);
    return file;
}

class FileManager {
    public:
        FileManager() = default;
        virtual ~FileManager() = default;

        virtual std::shared_ptr<std::istream> read(std::string path);

        virtual void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0);
        virtual void write(std::string path, SharedArray<uint8_t> buffer, bool append = false);

        virtual struct stat stat(std::string path);
        virtual SharedArray<std::string> list(std::string path);

        virtual void remove(std::string path);
        virtual void move(std::string path, std::string destination_path);
        virtual void copy(std::string path, std::string destination_path);
};
}
#endif