#include "heap.h"

// this extern just for verbosity and to tell reader this is exported, right?
extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode) {
    // traverse heap until satisfy min heap property. and child < right
    if (*root == NULL) {
        *root = newNode;
        return;
    }

    MemoryHeapNode * prev_node = NULL;
    MemoryHeapNode * node = *root;
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
    } 

    if (match) {
        // insert new node before matched node
        if (prev_node == NULL) {
            // root node...
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
    if (*root == NULL) {
        return NULL;
    }
    bool match = condition_func(node, condition_arg);
    MemoryHeapNode * prev_node = NULL;
    MemoryHeapNode * node = *root;
    while (!match) {
        if (node->child == NULL) {
            // no more to traverse, return NULL
            break;
        } else {
            prev_node = node;
            node = node->child;
        }
        match = condition_func(node, condition_arg);
    }

    if (match) {
        if (prev_node == NULL) {
            // root node...
            *root = node->child;
        } else {
            prev_node->child = node->child; // set parent child to current node child
        }
    } else {
        // return NULL if didn't find a matching node. else return the extracted node.
        node = NULL;
    }
    return node;
}