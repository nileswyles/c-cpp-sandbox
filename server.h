#ifndef SERVER_H
#include <stdint.h>

void server_listen(char * address, uint16_t port, void * handler_func);

#endif