#if defined __cplusplus
extern "C"
{
#endif

#ifndef SERVER_H
#define SERVER_H
#include <stdint.h>

typedef uint8_t(connection_handler_t)(int);

void server_listen(char * address, uint16_t port, connection_handler_t * handler);
#endif

#if defined __cplusplus
}
#endif
