#include "heap.h"

// this extern just for verbosity and to tell reader this is exported, right?

// fun fact, & pass by reference a cpp thing only? pointer * syntax more intuitive anyways... 
extern int memoryHeapPush(MemoryHeapNode ** root, MemoryHeapNode * newNode) {
    // traverse heap until satisfy min heap property. and left < right
    if (*root == NULL) {
        *root = newNode;
        return 1;
    }

    MemoryHeapNode * node = *root;
    // because min heap, 
    // if current_node > size, then 
    //  wtf even is a binary heap? 
    // hmmm... for now let's swap node with NewNode and make node child of newNode.
    // TODO: how to binary tree? because this will get unbalanced quickly?
    //      it's 
    if (node->block_size > newNode->block_size) {
        // TODO: enforce left < right, and maybe go left, right, left children? prioritize ehh probably not but interesting thought.
        newNode->left = node;
        newNode->parent = node->parent;
        if (node->parent->left == node) {
            node->parent->left = newNode;
        } else {
            node->parent->right = newNode; 

            // right node is currently only set during pops... :)
            //   hmm... actually doesn't work out that well because 
        }
        return 1;
    } 

    // else traverse tree
    if (node->left == NULL && node->right == NULL) { // if no children
        // TODO: balancing balancing balancing...
        //  non-binary heaps == sorted linked list... LMAO
        //      hashed array better where I can just do hash_func(size) round to nearest? in "constant" time lol
        //      but what would O(n) insert?

        // 
        node->left = newNode;
        return 1;
    }
    if (node->left != NULL) {
        int ret = push(node->left, newNode);
        if (ret == 1) { // means we found a place for this damn thing....
            return ret;
        }
    }
    if (node->right != NULL) {
        return push(node->right, newNode);
    }
}

// hmm... might need to update root right?
//  yes, 
extern MemoryHeapNode * memoryHeapPop(MemoryHeapNode ** root, HeapPopCondition condition_func, void * condition_arg) {
    if (*root == NULL) {
        return NULL;
    }
    MemoryHeapNode * node = *root;
    // traverse heap until size <= mem_node.block_size,
    // So, I think for this, since we are implementing a binary heap, we can assume, current_node < left < right?
    //  realistically we can grab any node as long as it satifies that request, but minimizing size minimizes fragmentation? 
    //      this might be too much lol

    // BIG O(log n) but since?
    if (condition_func(node, condition_arg)) {
        // if node satisfies condition of >size

        // TODO: actually remove the node.... LMAO
        //  is this reason for binary? lmao because when removed it collapses to two nodes LMAO... so lame....
        //  it is what it is :) suuuuuuper!

        // parent inherits children lmao...
        //  children get new parent...
        //  so we get left and right populated?

        //  or even better, promote child as parent? wowzers DEEP

        // nah, then it's the same problem if child has children...

        // since push isn't loading right anyways (linked list), for now...

        if (node->parent == NULL) {
            // root node...
            *root = node->left;
            node->left->parent = NULL;
        } else if (node->parent->left == node) {
            node->parent->left = node->left; // set parent child to current node child
            node->left->parent = node->parent; // set child parent to current node parent
        } else {}

        return node;
    } 

    // else traverse tree
    if (node->left == NULL && node->right == NULL) {
        return NULL;
    }
    MemoryHeapNode * ret_node = NULL;
    if (node->left != NULL) {
        ret_node = pop(node->left);
        if (ret_node != NULL) {
            return ret_node;
        }
    }
    if (node->right != NULL) {
        return pop(node->right);
    }
}