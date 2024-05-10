#include <stdint.h>
#include <stdio.h>
#include "emalloc.h"
#include "heap.h"

#include "logger.h"

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
    if (freedRootNode == NULL && usedRootNode == NULL) {
        logger_printf(LOGGER_DEBUG, "intializing heap... \n");

        nodes[0].index = 0;
        nodes[0].block_size = DYNAMIC_MEMORY_SIZE;
        nodes[0].child = NULL;
        memoryHeapPush(&freedRootNode, nodes);
        // logger_printf(LOGGER_DEBUG, "sizeof pointer, %u ", sizeof(uint8_t *)/2);
        // logger_printf(LOGGER_DEBUG, "sizeof size_t %u \n", sizeof(size_t)/2); // because 32-bit.... 
    }
}

extern void * emalloc(size_t size) {
    initializeData();

    if (size < 1) { 
        // freak out 
        return NULL;
    } 

    logger_printf(LOGGER_DEBUG, "Popping from freed heap. Root pointer: %p\n", freedRootNode);
    MemoryHeapNode * node = memoryHeapPop(&freedRootNode, sizeHeapPopCondition, &size);
    logger_printf(LOGGER_DEBUG, "Node pointer: %p, Root pointer: %p\n", node, freedRootNode);
    if (node == NULL) { 
        logger_printf(LOGGER_DEBUG, "Popped node is NULL, return NULL immediately.\n");
        return NULL;
    } 
    uint32_t node_index = node->index;
    if (node->block_size < size || 0 > node_index || node_index >= DYNAMIC_MEMORY_SIZE) { 
        return NULL;
    }
    logNodeContents(node);
    void * extracted_ptr = (void *)(dynamic_memories + node_index);
    if (node->block_size == size) {
        logger_printf(LOGGER_DEBUG, "Pushing to used heap, same size.\n");
        memoryHeapPush(&usedRootNode, node);
    } else { 
        // see check above...
        // if (node->block_size > size) {
        uint32_t new_index = node_index + size;
        nodes[new_index].index = new_index;
        nodes[new_index].block_size = node->block_size - size;
        nodes[new_index].child = NULL;
     
        // re-insert updated node (in freed list), with new size...
        logger_printf(LOGGER_DEBUG, "Pushing to freed heap.\n");
        logNodeContents(nodes + new_index);
        memoryHeapPush(&freedRootNode, nodes + new_index);

        nodes[node_index].index = node_index;
        nodes[node_index].block_size = size;
        nodes[node_index].child = NULL;

        logger_printf(LOGGER_DEBUG, "Pushing to used heap.\n");
        logNodeContents(nodes + node_index);
        memoryHeapPush(&usedRootNode, nodes + node_index);
    }
    logger_printf(LOGGER_DEBUG, "Returning pointer: %p, size: %lu\n", extracted_ptr, size);
    return extracted_ptr;
}

extern void efree(void * ptr) {
    uint32_t index = (uint8_t *)ptr - (uint8_t *)dynamic_memories;
    logger_printf(LOGGER_DEBUG, "Freeing pointer: %p, node index: %u\n", ptr, index);
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