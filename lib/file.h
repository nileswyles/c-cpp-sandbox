#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "reader.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace WylesLibs::File {

static reader * read(const char * file_path) {
    int fd = open(file_path, O_READONLY);
    reader * reader = reader_constructor(fd, READER_RECOMMENDED_BUF_SIZE);
    // so I am not a complete retard...
    reader * copy = reader_read_until(reader, (char)EOF);
    reader_destructor(reader);
    return copy;
}

}
#endif