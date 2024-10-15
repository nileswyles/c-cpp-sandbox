#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "iostream/iostream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>

#include <fstream>
#include <sys/stat.h>

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

static void write(std::string file_path, SharedArray<uint8_t> buffer, bool append) {
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
        s.write((const char *)buffer.buf(), buffer.size()); // binary output
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

}
#endif