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
static MemoryHeapNode * freed_root_node = NULL;
static MemoryHeapNode * used_root_node = NULL;

static inline void initializeData() {
    if (freed_root_node == NULL && used_root_node == NULL) {
        loggerPrintf(LOGGER_DEBUG, "Intializing heap... \n");

        nodes[0].index = 0;
        nodes[0].block_size = DYNAMIC_MEMORY_SIZE;
        nodes[0].child = NULL;
        memoryHeapPush(&freed_root_node, nodes);
    }
}

extern void * emalloc(size_t size) {
    initializeData();

    if (size < 1) return NULL;

    loggerPrintf(LOGGER_DEBUG, "Popping from freed heap. Root pointer: %p\n", freed_root_node);
    MemoryHeapNode * node = memoryHeapPop(&freed_root_node, sizeHeapPopCondition, &size);
    if (node == NULL) return NULL;
    loggerPrintf(LOGGER_DEBUG, "Node pointer: %p, Root pointer: %p\n", node, freed_root_node);

    uint32_t used_index = node->index;
    if (node->block_size < size || 0 > used_index || used_index >= DYNAMIC_MEMORY_SIZE) return NULL;

    memoryLogNodeContents(node);

    void * extracted_ptr = (void *)(dynamic_memories + used_index);
    if (node->block_size == size) {
        loggerPrintf(LOGGER_DEBUG, "Pushing to used heap, same size.\n");
        memoryHeapPush(&used_root_node, node);
    } else { 
        // see check above...
        // if (node->block_size > size) {
        uint32_t freed_index = used_index + size;
        nodes[freed_index].index = freed_index;
        nodes[freed_index].block_size = node->block_size - size;
        nodes[freed_index].child = NULL;
     
        // re-insert updated node (in freed list), with new size...
        loggerPrintf(LOGGER_DEBUG, "Pushing to freed heap.\n");
        memoryLogNodeContents(nodes + freed_index);
        memoryHeapPush(&freed_root_node, nodes + freed_index);

        nodes[used_index].index = used_index;
        nodes[used_index].block_size = size;
        nodes[used_index].child = NULL;

        loggerPrintf(LOGGER_DEBUG, "Pushing to used heap.\n");
        memoryLogNodeContents(nodes + used_index);
        memoryHeapPush(&used_root_node, nodes + used_index);
    }

    loggerPrintf(LOGGER_DEBUG, "Returning pointer: %p, size: %lu\n", extracted_ptr, size);

    return extracted_ptr;
}

extern void efree(void * ptr) {
    uint32_t index = (uint8_t *)ptr - (uint8_t *)dynamic_memories;
    loggerPrintf(LOGGER_DEBUG, "Freeing pointer: %p, node index: %u\n", ptr, index);

    if (0 <= index && index < DYNAMIC_MEMORY_SIZE) {
        MemoryHeapNode * freed = memoryHeapPop(&used_root_node, ptrHeapPopCondition, &index);
        MemoryHeapNode * found_contigious = memoryHeapPop(&freed_root_node, mergeHeapPopCondition, &index);
        if (found_contigious == NULL) {
            memoryHeapPush(&freed_root_node, freed);
        } else {
            found_contigious->block_size += freed->block_size;
            memoryHeapPush(&freed_root_node, found_contigious);
        }
        // yeah, this is actually pretty clean! I like the solution, assuming that mergeHeadPopCondition stuff works.
    }
}