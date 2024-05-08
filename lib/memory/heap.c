#include "heap.h"
#include "stdio.h"

// this extern just for verbosity and to tell reader this is exported, right?
extern void memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode) {
    // traverse heap until satisfy min heap property. and child < right
    if (*root == NULL) {
        *root = newNode;
        printf("EMPTY HEAP! root_index: %u, root_block_size: %u\n", (*root)->index, (*root)->block_size);
        return;
    }

    printf("HEAP! newNode_index: %u, newNode_block_size: %u\n", newNode->index, newNode->block_size);
    printf("HEAP! root_index: %u, root_block_size: %u\n", (*root)->index, (*root)->block_size);

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
            printf("Root Node reset to newNode\n");
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
    MemoryHeapNode * node = *root;
    MemoryHeapNode * prev_node = NULL;
    bool match = condition_func(node, condition_arg);
    while (!match) {
        printf("POP! node_index: %u, node_block_size: %u\n", node->index, node->block_size);
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
        printf("POP! node_index: %u, node_block_size: %u\n", node->index, node->block_size);
        if (prev_node == NULL) {
            // root node...
            printf("Root Node reset to node->child which could be null.\n");
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