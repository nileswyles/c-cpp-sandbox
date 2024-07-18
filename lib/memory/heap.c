#include "heap.h"
#include "stdio.h"

#include "logger.h"

extern bool sizeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    uint32_t size = *((uint32_t *)arg);
    loggerPrintf(LOGGER_DEBUG, "Size arg: %u, Block Size: %u\n", size, node->block_size);
    if (size <= node->block_size) {
        return true;
    }
    return false;
}

extern bool ptrHeapPopCondition(MemoryHeapNode * node, void * arg) {
    uint32_t index = *((uint32_t *)arg);
    loggerPrintf(LOGGER_DEBUG, "Popping node at index: %u\n", index);
    loggerPrintf(LOGGER_DEBUG, "\tCurrent node: %u\n", node->block_size);
    if (index == node->index) {
        return true;
    }
    return false;
}

extern bool mergeHeapPopCondition(MemoryHeapNode * node, void * arg) {
    uint32_t index = *((uint32_t *)arg);
    loggerPrintf(LOGGER_DEBUG, "Popping (contigious) node, preceding the node at index: %u\n", index);
    loggerPrintf(LOGGER_DEBUG, "\tCurrent node: %u, Block Size: %u\n", node->index, node->block_size);
    if (node->index + node->block_size == index) {
        return true;
    }
    return false;
}

extern void memoryLogNodeContents(MemoryHeapNode * node) {
    loggerPrintf(LOGGER_DEBUG, "  node->index: %u, node->block_size: %u\n", node->index, node->block_size);
}

// this extern just for verbosity and to tell reader this is exported, right?
extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode) {
    MemoryHeapNode * prev_node = NULL;
    MemoryHeapNode * node = *root;
    if (*root == NULL) {
        *root = newNode;
        return;
    }
    loggerPrintf(LOGGER_DEBUG, "Root Node:\n");
    memoryLogNodeContents(node); 
    loggerPrintf(LOGGER_DEBUG, "New Node:\n");
    memoryLogNodeContents(newNode); 

    bool match = node->block_size > newNode->block_size;
    while (!match) {
        if (node->child == NULL) {
            // no more to traverse, return break out of loop.
            break;
        } else {
            prev_node = node;
            node = node->child;
        }
        match = node->block_size > newNode->block_size;
        loggerPrintf(LOGGER_DEBUG, "Current Node:\n");
        memoryLogNodeContents(node); 
    } 

    if (match) {
        // insert new node before matched node
        if (prev_node == NULL) {
            // root node...
            loggerPrintf(LOGGER_DEBUG, "Root Node reset to newNode\n");
            *root = newNode;
        } else {
            prev_node->child = newNode;
        }
        newNode->child = node;
    } else {
        // broke out of loop without match...
        //  append...
        node->child = newNode;
        // make sure it's NULL, since we are appending to end of list.
        newNode->child = NULL;
    }
}

extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func, void * condition_arg) {
    MemoryHeapNode * node = *root;
    MemoryHeapNode * prev_node = NULL;
    if (*root == NULL) {
        return NULL;
    }
    loggerPrintf(LOGGER_DEBUG, "Root Node:\n");
    memoryLogNodeContents(*root); 

    bool match = condition_func(node, condition_arg);
    while (!match) {
        if (node->child == NULL) {
            // no more to traverse, return NULL
            break;
        } else {
            prev_node = node;
            node = node->child;
        }
        match = condition_func(node, condition_arg);
        loggerPrintf(LOGGER_DEBUG, "Current Node:\n");
        memoryLogNodeContents(node); 
    }

    if (match) {
        loggerPrintf(LOGGER_DEBUG, "Found match! node_index: %u, node_block_size: %u\n", node->index, node->block_size);
        if (prev_node == NULL) {
            // root node...
            loggerPrintf(LOGGER_DEBUG, "Root Node reset to node->child which could be null.\n");
            *root = node->child;
        } else {
            prev_node->child = node->child; // set parent child to current node child
        }
        node->child = NULL;
    } else {
        // return NULL if didn't find a matching node. else return the extracted node.
        loggerPrintf(LOGGER_DEBUG, "No match!\n");
        node = NULL;
    }

    return node;
}