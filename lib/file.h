#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace WylesLibs::File {

static Array * read(const char * file_path) {
    // because VLA?
    const size_t buf_size = 1024;

    int fd = open(file_path, O_RDONLY);
    reader r;
    uint8_t buf[buf_size];
    reader_initialize(&r, buf, fd, buf_size);
    return reader_read_until(&r, (char)EOF);
}

}
#endif