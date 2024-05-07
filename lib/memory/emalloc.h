#ifndef EMALLOC_H
#define EMALLOC_H

#include <stdbool.h>

// right?
#define malloc emalloc

#define DYNAMIC_MEMORY_SIZE 2 << 32;

// TODO: then add support for virtual memory addresses (MMU's?, paging, swap?)
static void DYNAMIC_MEMORIES[DYNAMIC_MEMORY_SIZE] = {};

static TreeNode nodes[DYNAMIC_MEMORY_SIZE] = {}; 

nodes[0] = { 
    .data_ptr = DYNAMIC_MEMORIES, 
    .block_size = DYNAMIC_MEMORY_SIZE, 
    .parent = NULL, .left = NULL, .right = NULL 
};
TreeNode * freedRootNode = nodes[0];

// TODO: can initialize like this to make things easier?
//  nah, I can't LMAO
TreeNode * usedRootNode = { 
    .data_ptr = NULL, 
    .block_size = 0, 
    .parent = NULL, .left = NULL, .right = NULL 
};

// yeah, thinking about this some more might seldom use this but good to have? 
//  without any paging, this is fucking dumb :)
extern void * emalloc(size_t size) {
    // hmmm... yeah so 
    TreeNode * node = memoryHeapPop(freedRootNode, size);
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
        memoryHeapPush(usedRootNode, node);
    } else { // if (node->block_size > size) {
        size_t nodes_index = extracted_ptr - DYNAMIC_MEMORIES;
        // TODO: I think, this will always be within bounds but revisit...
        node->ptr += size;
        node->block_size -= size;
 
        // re-insert updated node (in freed list), with new size...
        memoryHeapPush(freedNode, node);

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
        memoryHeapPush(usedRootNode, nodes[nodes_index]);
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

    TreeNode * freed = memoryHeapPop(usedRootNode, ptrHeapPopCondition, &ptr);

    // hmm... now this next part
    // iterate over node array? that's an option...
    // this means we'll go from nodes_index -> 0; which can be a lot of iterations.

    // search for pointer < ptr where pointer_size + ptr - pointer == ptr.

    // memPopCpp2... function pointer condition it is :)

    // TODO: lmao, follow code guidelines 
    //  LMAO LMAO LMAO LMAO sajnklsnjkalndjslknadljknskladjk
    TreeNode * found_contigious = memoryHeapPop(freedRootNode, mergeHeapPopCondition, &ptr);

    if (found_contigious == NULL) {
        memoryHeapPush(freedRootNode, freed);
    } else {
        found_contigious->block_size += freed->size;
        memoryHeapPush(freedRootNode, found_contigious);
    }

    // yeah, this is actually pretty clean! I like the solution, assuming that mergeHeadPopCondition stuff works.
}

// More TODOS:
//  append, insert and remove functions for statically allocated arrays...

//  think about ring buffer backed linked list and heap structure... and whether that was even a good decision...

//  hmm... what data structure does the linux kernel use (not in glibc right?)?... doesn't necessarily have to be a heap... 
//      and actually most definitely not a heap

//  arrays obviously the most straight forward implementation
//      append set value at current position.
//      remove iterate until position mv anything after by 1 position.

//  benefit of psuedo linked-list heap structure is that it takes more time to insert, to speed up removals because sorted... (LMAO)
//      so there's something there, I guess....
//      now, when we "remove" an element from said structure, what do we want to happen to the underlying data structure... 

//      Nothing? Yeah, we just move it from used to freed structure and so we won't ever "lose" the pointer and it gets automatically reused...

// regardless of which data structure is chosen for used/freed nodes, we'll need to allocate storage for the maximum number of nodes possible which is the same as dynamic memory size....
//  so, continuing onward.

#endif