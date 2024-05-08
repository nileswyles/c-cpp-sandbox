#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

// # pointer arithmetic! let's remember to compile this separately... because elevated privileges are a thing... 

// #include <stdbool.h>
// TODO: is extern static a C linkage thing ? forreal? 
//  declaration added to source files through includes right and so it looks for source file with extern...

// heaps aka priority queues, so will likely need to decouple again... or might be worth implementing one in Cpp with templates?

// using array indexing instead of pointers (linked list) might save quite a bit on storage.
//  alternate between child most and right most
//  [5,4,2,1,3,6,7]

// Singly linked priority list data structure of memory nodes.
typedef struct MemoryHeapNode {
    void * ptr;
    size_t block_size;
    MemoryHeapNode * child;
} MemoryHeapNode;

static bool sizeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // if (size <= node->block_size)
    if ((size_t)*arg <= node->block_size) {
        return true;
    }
    return false;
}

static bool ptrHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // seems odd to cast like this but trust me?
    if ((void *)*arg <= node->block_size) {
        return true;
    }
    return false;
}

static bool mergeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    void * ptr = (void *)*arg;
    
    // search for node->ptr < ptr and node->ptr + node->block_size == ptr.
    if (ptr < node->ptr && node->ptr + node->block_size == ptr) {
        return true;
    }
    return false;
}

typedef bool(HeapPopCondition)(MemoryHeapNode *, void *);

extern int memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode);
extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func);

#endif 

