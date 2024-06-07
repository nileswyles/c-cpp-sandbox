#ifndef SERVER_H
#define SERVER_H

#if defined __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define SERVER_MINIMUM_CONNECTION_SPEED 131072 // 128kbps

typedef uint8_t(connection_handler_t)(int);

extern void serverListen(const char * address, const uint16_t port, connection_handler_t * handler);
extern void serverSetConnectionTimeout(int fd, uint32_t timeout_s);

#if defined __cplusplus
}
#endif

#endif
