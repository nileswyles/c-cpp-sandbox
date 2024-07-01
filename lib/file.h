#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader/reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>

#include <fstream>

using namespace WylesLibs;

namespace WylesLibs::File {

static void writeFile(std::string file_path, std::string buffer, bool append) {
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

static void writeFile(std::string file_path, Array<uint8_t> buffer, bool append) {
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
// TODO: seek... when multipart file support is added?
//  # ifstream - ed edition lol
//  Worth creating abstraction around that for files, instead of using reader? 
static WylesLibs::Array<uint8_t> read(std::string file_path) {
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1) {
        //  value in distinguishing between size == 0 vs uninitialized...?
        //      or return pointers to array and nullptr?
        //  in other words, non-existing file == empty file?

        // something to keep in mind, I guess... no changes needed at the moment. 
        return WylesLibs::Array<uint8_t>();
    }
    Reader r(fd);
    printf("???????\n");
    Array<uint8_t> file = r.readUntil((char)EOF, true);
    close(fd);
    return file;
}

}
#endif