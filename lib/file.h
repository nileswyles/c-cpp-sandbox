#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace WylesLibs::File {

// why is it trying to use that namespace?
// TODO: update reader to use new Array class... not just return type but buf obj too?
static WylesLibs::Array<uint8_t> * read(const char * file_path) {
    // because VLA?
    const size_t buf_size = 1024;

    int fd = open(file_path, O_RDONLY);
    Reader r(fd, buf_size);
    return r.readUntil((char)EOF);
}

}
#endif