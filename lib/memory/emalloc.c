#include "emalloc.h"

// TODO: then add support for virtual memory addresses (MMU's?, paging, swap?)
static void DYNAMIC_MEMORIES[DYNAMIC_MEMORY_SIZE] = {};
static MemoryHeapNode nodes[DYNAMIC_MEMORY_SIZE] = {}; 

nodes[0].data_ptr = DYNAMIC_MEMORIES;
nodes[0].block_size = DYNAMIC_MEMORY_SIZE;
nodes[0].child = NULL;

// use heap push function to intialize root
static MemoryHeapNode * freedRootNode = NULL;
memoryHeapPush(&freedRootNode, nodes);

// use heap push function to intialize root
static MemoryHeapNode * usedRootNode = NULL;

extern void * emalloc(size_t size) {
    if (size < 1) { 
        // freak out 
        return NULL;
    } 

    MemoryHeapNode * node = memoryHeapPop(&freedRootNode, sizeHeapPopCondition, &size);
    if (node == NULL || node->block_size < size) { 
        // freak out 
        return NULL;
    } 

    void * extracted_ptr = node->ptr;
    if (node->block_size == size) {
        memoryHeapPush(&usedRootNode, node);
    } else { 
        size_t nodes_index = extracted_ptr - DYNAMIC_MEMORIES;

        // see check above...
        // if (node->block_size > size) {
        node->ptr += size;
        node->block_size -= size;
 
        // re-insert updated node (in freed list), with new size...
        memoryHeapPush(&freedRootNode, node);

        nodes[nodes_index].data_ptr = extracted_ptr;
        nodes[nodes_index].block_size = size;
        nodes[nodes_index].child = NULL;
  
        memoryHeapPush(&usedRootNode, nodes + nodes_index);
    }
    return extracted_ptr;
}

//   freed will find used block at address provided... 
//      then remove entry from used blocks list completely and add new freed entry merging any contigious blocks? 
//      or was that why I had requirement of also minimizing pointers lmao (fuck)...
extern void efree(void * ptr) {
    // does this work? lol

    // this better? lmao
    // void * ptr_forreal = ptr;
    // &ptr_forreal

    MemoryHeapNode * freed = memoryHeapPop(&usedRootNode, ptrHeapPopCondition, &ptr);

    // search for found_contigious where found_contigious->ptr < ptr and found_contigious->block_size + found_cointigious->ptr == ptr.
    MemoryHeapNode * found_contigious = memoryHeapPop(&freedRootNode, mergeHeapPopCondition, &ptr);

    if (found_contigious == NULL) {
        memoryHeapPush(&freedRootNode, freed);
    } else {
        found_contigious->block_size += freed->size;
        memoryHeapPush(&freedRootNode, found_contigious);
    }

    // yeah, this is actually pretty clean! I like the solution, assuming that mergeHeadPopCondition stuff works.
}