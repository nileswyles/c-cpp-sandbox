#ifndef MEMORY_HEAP_H
#define MEMORY_HEAP_H

// # pointer arithmetic! let's remember to compile this separately... because elevated privileges are a thing... 

// #include <stdbool.h>
// TODO: is extern static a C linkage thing ? forreal? 
//  declaration added to source files through includes right and so it looks for source file with extern...

typedef struct MemoryHeapNode {
    void * ptr;
    size_t block_size;
    MemoryHeapNode * parent;
    MemoryHeapNode * left;
    MemoryHeapNode * right;
} MemoryHeapNode;

extern bool sizeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // if (size <= node->block_size)
    if ((size_t)*arg <= node->block_size) {
        return true;
    }
    return false;
}

extern bool ptrHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // seems odd to cast like this but trust me?
    if ((void *)*arg <= node->block_size) {
        return true;
    }
    return false;
}

extern bool mergeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    // search for pointer < ptr where pointer_size + ptr - pointer == ptr.
    //  lol what ?
    void * ptr = (void *)*arg;

    // parenthesis to enforce order of operations because computer maths?

    if (ptr < node->ptr && node->ptr + node->block_size == ptr) {
        return true;
    }
    return false;
}

typedef bool(HeapPopCondition)(MemoryHeapNode *, void *);

extern int memoryHeapPush(MemoryHeapNode * node, MemoryHeapNode * newNode);
extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode * node, HeapPopCondition condition_func);

#endif 

