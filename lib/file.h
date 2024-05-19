#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader/reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace WylesLibs::File {

// why is it trying to use that namespace?
// TODO: update reader to use new Array class... not just return type but buf obj too?
static WylesLibs::Array<uint8_t> read(const char * file_path) {
    // because VLA?
    const size_t buf_size = 1024;

    printf("READ FILE....\n");
    int fd = open(file_path, O_RDONLY);
    // Reader r(fd, buf_size);
    Reader r(fd);
    printf("READ FILE....\n");
    return r.readUntil((char)EOF);
}

}
#endif