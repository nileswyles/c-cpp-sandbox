#pragma once

#if defined(_MSC_VER)

// ! IMPORTANT - This is required by FD based estreams.
//      Just stubbing these out. We only support IStreamEStream (stdlib backed stream) via Microsoft Compiler.
int write(int fd, void * b, size_t nbytes) {
    return 0;
}
int read(int fd, void * b, size_t nbytes) {
    return 0;
}

#endif