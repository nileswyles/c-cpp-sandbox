#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader/reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string>

namespace WylesLibs::File {

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
    return r.readUntil((char)EOF, true);
}

}
#endif