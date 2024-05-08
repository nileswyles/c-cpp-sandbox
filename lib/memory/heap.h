#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

#include <stddef.h>
#include <stdint.h>

// #include <stdbool.h>

// heaps aka priority queues, so will likely need to decouple again... or might be worth implementing one in Cpp with templates?
// Singly linked priority list data structure of memory nodes.
typedef struct MemoryHeapNode {
    uint16_t index;
    uint16_t block_size;
    MemoryHeapNode * child;
} MemoryHeapNode;

static bool sizeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // if (size <= node->block_size)
    uint32_t size = *((uint32_t *)arg);
    if (size <= node->block_size) {
        return true;
    }
    return false;
}

static bool ptrHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // seems odd to cast like this but trust me?
    uint32_t index = *((uint32_t *)arg);
    if (index == node->index) {
        return true;
    }
    return false;
}

static bool mergeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    uint32_t index = *((uint32_t *)arg);
    if (node->index + node->block_size == index) {
        return true;
    }
    return false;
}

typedef bool(HeapPopCondition)(MemoryHeapNode *, void *);

extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode);
extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func, void * condition_arg);

#endif 

