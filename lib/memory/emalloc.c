#include "emalloc.h"

// TODO: then add support for virtual memory addresses (MMU's?, paging, swap?)
static void DYNAMIC_MEMORIES[DYNAMIC_MEMORY_SIZE] = {};

static TreeNode nodes[DYNAMIC_MEMORY_SIZE] = {}; 
nodes[0] = { 
    .data_ptr = DYNAMIC_MEMORIES, 
    .block_size = DYNAMIC_MEMORY_SIZE, 
    .parent = NULL, .left = NULL, .right = NULL 
};

// use heap push function to intialize root
static TreeNode * freedRootNode = NULL;
memoryHeapPush(&freedRootNode, nodes[0]);

// use heap push function to intialize root
static TreeNode * usedRootNode = NULL;

// yeah, thinking about this some more might seldom use this but good to have? 
//  without any paging, this is fucking dumb :)
extern void * emalloc(size_t size) {
    // hmmm... yeah so 
    TreeNode * node = memoryHeapPop(&freedRootNode, size);
    // TODO:
    // input validation?

    // TODO:
    // get address from node, insert update node (only if block_size > size, else we remove...) in free structure...
    // TODO: might specialize the structure and remove the need for this 
    if (node == NULL || node->block_size < size) { 
        // freak out 
        return NULL;
    } 
    
    void * extracted_ptr = node->ptr;
    if (node->block_size == size) {
        // just move the node to the used list
        // LMAO
        memoryHeapPush(&usedRootNode, node);
    } else { // if (node->block_size > size) {
        size_t nodes_index = extracted_ptr - DYNAMIC_MEMORIES;
        // TODO: I think, this will always be within bounds but revisit...
        node->ptr += size;
        node->block_size -= size;
 
        // re-insert updated node (in freed list), with new size...
        memoryHeapPush(&freedRootNode, node);

        // create new node to add to used array...
        //      since there are finite number of addresses... our node buffers can be set to size of memory array and we'll never go past the limit.
        //      TODO: is this an acceptable tradeoff... hmm... actually might apply to the array implementation too... derp.

        // more abstraction
        // TODO: might want to use index of pointer in dynamic buffer as index for this node? 
        // create new used memory node.
        nodes[nodes_index] = { 
            .ptr = extracted_ptr, 
            .block_size = size, 
            .parent = NULL, .left = NULL, .right = NULL 
        };
  
        // TODO:
        //  Don't care about return here... at least not yet lol
        memoryHeapPush(&usedRootNode, nodes[nodes_index]);
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

    TreeNode * freed = memoryHeapPop(&usedRootNode, ptrHeapPopCondition, &ptr);

    // hmm... now this next part
    // iterate over node array? that's an option...
    // this means we'll go from nodes_index -> 0; which can be a lot of iterations.

    // search for pointer < ptr where pointer_size + ptr - pointer == ptr.

    // memPopCpp2... function pointer condition it is :)

    // TODO: lmao, follow code guidelines 
    //  LMAO LMAO LMAO LMAO sajnklsnjkalndjslknadljknskladjk
    TreeNode * found_contigious = memoryHeapPop(&freedRootNode, mergeHeapPopCondition, &ptr);

    if (found_contigious == NULL) {
        memoryHeapPush(&freedRootNode, freed);
    } else {
        found_contigious->block_size += freed->size;
        memoryHeapPush(&freedRootNode, found_contigious);
    }

    // yeah, this is actually pretty clean! I like the solution, assuming that mergeHeadPopCondition stuff works.
}