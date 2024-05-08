#ifndef EMALLOC_H
#define EMALLOC_H

#include <stdbool.h>
#include <stddef.h>

// right?
#define malloc emalloc

#ifndef DYNAMIC_MEMORY_SIZE
#define DYNAMIC_MEMORY_SIZE 65536
#endif

extern void * emalloc(size_t size);
extern void efree(void * ptr);

#endif