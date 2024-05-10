#ifndef EMALLOC_H
#define EMALLOC_H

#include <stdbool.h>
#include <stddef.h>

#define malloc emalloc
#define free efree

// total size required =
//  (2 + 2 + 4) * 65536
//  +
//  65536

//  sizeof(MemoryHeapNode) * DYNAMIC_MEMORY_SIZE + DYNAMIC_MEMORY_SIZE

//  (size_index + size_block_size + pointer_size(32/64 bits)) * DYNAMIC_MEMORY_SIZE
//      +
//  DYNAMIC_MEMORY_SIZE
#ifndef DYNAMIC_MEMORY_SIZE
#define DYNAMIC_MEMORY_SIZE 65536
#endif

extern void * emalloc(size_t size);
extern void efree(void * ptr);

#endif