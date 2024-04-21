#ifndef SERVER_H
#define SERVER_H

#if defined __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef uint8_t(connection_handler_t)(int);

extern void server_listen(const char * address, const uint16_t port, connection_handler_t * handler);

#if defined __cplusplus
}
#endif

#endif
