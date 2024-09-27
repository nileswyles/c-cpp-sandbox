#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader/reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>

#include <fstream>
#include <sys/stat.h>

#include "transport.h"

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
    } else {
        if (append) {
            s.seekp(0, std::ios_base::end);
        } else {
            s.seekp(0);
        }
        s.write((const char *)buffer.data(), buffer.size()); // binary output
    }
}

static void write(std::string file_path, Array<uint8_t> buffer, bool append) {
    // open every time a problem?
    std::fstream s{file_path, s.binary | s.out};
    if (!s.is_open()) {
    } else {
        if (append) {
            s.seekp(0, std::ios_base::end);
        } else {
            s.seekp(0);
        }
        s.write((const char *)buffer.buf(), buffer.size()); // binary output
    }
}

static WylesLibs::Array<uint8_t> read(std::string file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        //  value in distinguishing between size == 0 vs uninitialized...?
        //      or return pointers to array and nullptr?
        //  in other words, non-existing file == empty file?

        // something to keep in mind, I guess... no changes needed at the moment. 
        return WylesLibs::Array<uint8_t>();
    }
    struct stat stat_info = {};
    int lol = fstat(fd, &stat_info);
    Transport io(fd);
    Reader r(&io);
    Array<uint8_t> file = r.readBytes(stat_info.st_size);
    close(fd);
    return file;
}

}
#endif