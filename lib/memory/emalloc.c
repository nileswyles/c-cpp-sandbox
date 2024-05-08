#include <stdint.h>
#include <stdio.h>
#include "emalloc.h"
#include "heap.h"

// TODO: then add support for virtual memory addresses (MMU's?, paging, swap?)

// let's assume byte addressable for now? yeah, most is probably byte addressable... who word addresses?

// TODO:
// I have a feeling this information is available somehow... but for now let's assume byte addressable 

static uint8_t dynamic_memories[DYNAMIC_MEMORY_SIZE] = {0};
static MemoryHeapNode nodes[DYNAMIC_MEMORY_SIZE] = {0}; 

// use heap push function to intialize root
static MemoryHeapNode * freedRootNode = NULL;
// use heap push function to intialize root
static MemoryHeapNode * usedRootNode = NULL;

static inline void initializeData() {
    if (freedRootNode == NULL) {
        nodes[0].index = 0;
        nodes[0].block_size = DYNAMIC_MEMORY_SIZE;
        nodes[0].child = NULL;
        memoryHeapPush(&freedRootNode, nodes);
        
        // printf("sizeof pointer, %u ", sizeof(uint8_t *)/2);
        // printf("sizeof size_t %u \n", sizeof(size_t)/2); // because 32-bit.... 
    }
}

extern void * emalloc(size_t size) {
    initializeData();

    if (size < 1) { 
        // freak out 
        return NULL;
    } 

    MemoryHeapNode * node = memoryHeapPop(&freedRootNode, sizeHeapPopCondition, &size);
    uint32_t nodes_index = node->index;
    if (node == NULL || node->block_size < size || 
            0 > nodes_index || nodes_index >= DYNAMIC_MEMORY_SIZE) { 
        // freak out 
        return NULL;
    } 
    void * extracted_ptr = (void *)(dynamic_memories + nodes_index);
    if (node->block_size == size) {
        memoryHeapPush(&usedRootNode, node);
    } else { 

        // see check above...
        // if (node->block_size > size) {
        node->index += size;
        node->block_size -= size;
 
        // re-insert updated node (in freed list), with new size...
        memoryHeapPush(&freedRootNode, node);

        nodes[nodes_index].index = nodes_index;
        nodes[nodes_index].block_size = size;
        nodes[nodes_index].child = NULL;
  
        memoryHeapPush(&usedRootNode, nodes + nodes_index);
    }
    return extracted_ptr;
}

extern void efree(void * ptr) {
    uint32_t index = (uint8_t *)ptr - (uint8_t *)dynamic_memories;
    if (0 <= index && index < DYNAMIC_MEMORY_SIZE) {
        MemoryHeapNode * freed = memoryHeapPop(&usedRootNode, ptrHeapPopCondition, &index);
        MemoryHeapNode * found_contigious = memoryHeapPop(&freedRootNode, mergeHeapPopCondition, &index);
        if (found_contigious == NULL) {
            memoryHeapPush(&freedRootNode, freed);
        } else {
            found_contigious->block_size += freed->block_size;
            memoryHeapPush(&freedRootNode, found_contigious);
        }
        // yeah, this is actually pretty clean! I like the solution, assuming that mergeHeadPopCondition stuff works.
    }
}