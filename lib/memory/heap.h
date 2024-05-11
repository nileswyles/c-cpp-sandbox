#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "logger.h"

// #include <stdbool.h>

// heaps aka priority queues, so will likely need to decouple again... or might be worth implementing one in Cpp with templates?
// Singly linked priority list data structure of memory nodes.
typedef struct MemoryHeapNode {
    uint16_t index;
    uint16_t block_size;
    MemoryHeapNode * child;
} MemoryHeapNode;

extern void memoryLogNodeContents(MemoryHeapNode * node);

extern bool sizeHeapPopCondition(MemoryHeapNode * node, void * arg);
extern bool ptrHeapPopCondition(MemoryHeapNode * node, void * arg);
extern bool mergeHeapPopCondition(MemoryHeapNode * node, void * arg);

typedef bool(HeapPopCondition)(MemoryHeapNode *, void *);

extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode);
extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func, void * condition_arg);


#endif 

