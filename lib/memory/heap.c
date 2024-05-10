#include "heap.h"
#include "stdio.h"

// this extern just for verbosity and to tell reader this is exported, right?
extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode) {
    MemoryHeapNode * prev_node = NULL;
    MemoryHeapNode * node = *root;
    if (*root == NULL) {
        *root = newNode;
        return;
    }
    logger_printf(LOGGER_DEBUG, "Root Node:\n");
    logNodeContents(node); 
    logger_printf(LOGGER_DEBUG, "New Node:\n");
    logNodeContents(newNode); 

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
        logger_printf(LOGGER_DEBUG, "Current Node:\n");
        logNodeContents(node); 
    } 

    if (match) {
        // insert new node before matched node
        if (prev_node == NULL) {
            // root node...
            logger_printf(LOGGER_DEBUG, "Root Node reset to newNode\n");
            *root = newNode;
        } else {
            prev_node->child = newNode;
        }
        newNode->child = node;
    } else {
        // broke out of loop without match...
        //  append...
        node->child = newNode;
    }
}

extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func, void * condition_arg) {
    MemoryHeapNode * node = *root;
    MemoryHeapNode * prev_node = NULL;
    if (*root == NULL) {
        return NULL;
    }
    logger_printf(LOGGER_DEBUG, "Root Node:\n");
    logNodeContents(*root); 

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
        logger_printf(LOGGER_DEBUG, "Current Node:\n");
        logNodeContents(node); 
    }

    if (match) {
        logger_printf(LOGGER_DEBUG, "Found match! node_index: %u, node_block_size: %u\n", node->index, node->block_size);
        if (prev_node == NULL) {
            // root node...
            logger_printf(LOGGER_DEBUG, "Root Node reset to node->child which could be null.\n");
            *root = node->child;
        } else {
            prev_node->child = node->child; // set parent child to current node child
        }
    } else {
        // return NULL if didn't find a matching node. else return the extracted node.
        logger_printf(LOGGER_DEBUG, "No match!\n");
        node = NULL;
    }
    return node;
}