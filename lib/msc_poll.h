#pragma once

#if defined(_MSC_VER)

// ! IMPORTANT - This is required by FD based estreams.
//      Just stubbing these out. We only support IStreamEStream (stdlib backed stream) via Microsoft Compiler.
#define POLLIN 0

typedef int nfds_t;
typedef struct pollfd {
    int fd;
    short events;
    short revents;
};

int poll(struct pollfd* __fds, nfds_t __nfds, int __timeout);

#endif