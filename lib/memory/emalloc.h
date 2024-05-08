#ifndef EMALLOC_H
#define EMALLOC_H

#include <stdbool.h>

// right?
#define malloc emalloc

#ifndef DYNAMIC_MEMORY_SIZE
#define DYNAMIC_MEMORY_SIZE 2 << 16;
#endif

extern void * emalloc(size_t size);
extern void efree(void * ptr);

#endif