#ifndef EMALLOC_H
#define EMALLOC_H

// right?
#define malloc emalloc

// Working through problem, let's understand memory management...:
//      because zephyr obviously supports non-A (cortex-M/R) arm cores.  

// some datastructure containing physical addresses (divied up in units of size_t?)
//  some 

// map of addresses allocated created.
//  <starting address?> : {all other addresses}

//  free(starting address) also clears other addresses, 
//     but freeing other addresses does what? Just that address? 
//      Might be simpler or only make sense to make that address available if the Main one is freed :) because the fact that we want them to be contigious?

// how do we defrag / preserve contigiousness, need some mmu for that? or can I implement something in firm/soft/ware.
// 
//  might need some mmu otherwise, can run into allocation issues very quickly... 
//      can probably constrain the problem to providing dynamic memory only if a contigious block of memory exists.
//      let's start there for now...


// given the above, malloc might need to do the following (assuming an array data structure?, idk):
//  iterate over list of start of free addresses
//  maybe this is already associated with an end address then you can do ptr maths to get size of block? or even better, it's already stored in some structure.


//  Assuming an already fragmented memory layout
//      you would need such a data structure in the following way:
//          "maintain" sorted list of either free or used addresses (ascending order). - free addresses more than likely.
//          as mentioned before, we want contigious memory, so we might not select the first pointer available but the smallest starting addresses who's "length" (you get what I mean) is at least size passed into malloc.   
//          a heap datastructure organizes these addresses in a way that makes that very easy to compute.
//          with a min heap you just grab the left most leg of the tree... :)

// Okay, I understand that min heap is the best data structure for it but I sort of had prior knowledge. How does one arrive there without that knowledge?

//  so you have physical addresses of which some might be free'd and some used

//      Might want to simplify to just malloc's no free's?
    //      If all are freed then malloc can grab smallest address... and as simple as keeping a running pointer incremented by size every time malloc is called. 

//  okay, so now if you free a block of memory, then you might want to reassign that block of memory before using newer blocks of memory?
//      you can continue this path and arrive at requiring some sort of heap(like) data structure for optimal runtime performance. becuase we limit search space? 
//      if so, then a heap is definetly the way to go? because that left most leg of the tree would represent taht most recently freed block...


//  LIMIT SEARCH SPACE! LIMIT SEARCH SPACE? Let's go down this rabbit hole... it might be worth it... oooo a piece of candy :)
//      given this finite set of paths (graph theory term for this?) I think I am likkely to converge at a an optimal solution where there aren't any real trade offs. but anyways

// Do you really though? Let's play devil's advocate....
//  So, what if we just ignore that freed block of memory until absolutely needed? LMAO
//      We continue using that running pointer of incremented by size every malloc call... 
//      then you reach end, so you need some policy for dealing with this (so back to previous question? lol, so use heap?, let's continue anyways)
//          might just end up describing how to build the heap structure described above (on the fly)...

//
//      maybe you'll keep a list of freed blocks? so something like {address, size } (because why store all possible addresses, just compress with size, since we know it's a "block", sometimes it's good).... 
//           this list of freed blocks will ideally be already sorted when malloc is called, so when freed? or do we just iterate each time over list of blocks each time and get min address? 
//           there might something to prioritizing malloc's over free's? (yet another path divergence - trade off?)... 
//              furthermore, freed blocks might be initialized to {start of memory, size of memory}. 
//              Then you allocate some memory of size 27, 
//              {27, size of memory - 27}, 
//              Then you allocate some memory of size 17, 
            //  {17 + 27, size of memory - 27 - 17}
//              Then you allocate some memory of size 7, 
            //  {17 + 27 + 7, size of memory - 27 - 17 - 7}

            // Then free block of size 17 :), will add another entry to freed list as such
            //  {17 + 27 + 7, size of memory - 27 - 17 - 7}, {27, 17}

            // Then you realize, the free operation requires a map of used blocks {address, size}... so maybe it's the same struct you just move locations...

            // then continue with that algorithm... 

            // most "straight forward"? lol solution,
            //   malloc will iterate over freed blocks, find address where size > new_size;
            //   update/move freed block and/or add used block.

            //   freed will find used block at address provided... 
            //      then remove entry from used blocks list completely and add new freed entry merging any contigious blocks? 
            //      or was that why I had requirement of also minimizing pointers lmao (fuck)...

            //   and so, no need to iterate more than length of lists described above.

            //   so not really a heap lol just a list of used and freed blocks...
            //   tradeoffs, heap insert actually more than worth for this problem?
            //
            //   hmm... seems there's a lot more to go... (first let's confirm the solution described above actually works.)
            //      practice makes better? but not best?

            //      what's faster than BIG O(n)? 
            //      O(log n) and O(1)

            //      log's typically mean we split search space in half periodically... right? yeah, log base 2.... lol

            // good to remember definitions.
            //  heap == parent statisfies min/max of child... You can have as many children... but in this case maybe just 1 child... () 
            //  an array of contigious addresses just a special heap?

            // 2 children

            //  let's say we have a heap data structure...
            //      
            //      binary min heap == log n inserts and constant lookup... that's what we want.
            //      so two heaps...
            //      so, that's the most optimal solution? but let's rewind for a sec.

            //      O(1), wtf even is that? Hashes?


//  So thought about it as a list (ring buffer?) but let's implement as a heap?...
// need a ring buffer too lol no?

typedef struct TreeNode {
    void * data_ptr;
    TreeNode * parent;
    TreeNode * left;
    TreeNode * right;
} TreeNode;
typedef struct MemNode {
    void * ptr,
    size_t block_size,
} MemNode;
typedef struct SArray {
    // FIFO not stack, because seems more useful? (applicable to more situations)
    size_t push;
    size_t pop;
    size_t cap;
} SArray;


// return 1, if looped around, so that user knows and can do whatever it wants...
//  after calling this function, push will point to most recently added item, pop will point to oldest item?
static void sArrayInit(SArray * buffer, size_t cap) {
    buffer->push = -1;
    buffer->pop = -1;
    buffer->cap = cap;
}
// -1, -1
//  0, -1 - push - size = 1
//  1, -1 - push - size = 2
//  1, 0  - pop - size = 1
//  1, 1  - pop - size = 0
static size_t sArrayAdd(SArray * buffer) {
    // push always and remove oldest element if needed lol
    if (++buffer->push > buffer->cap) {
        buffer->push = 0;
    }
    if (buffer->push == buffer->pop) {
        buffer->pop++;
        return 1; // return 1 if removed oldest element
    }
    return 0;
}
static void sArrayRemove(SArray * buffer) {
    // if removed all elements, do nothing... else increment pop buffer and loop if needed.
    if (buffer->push != buffer->pop) {
        if (++buffer->pop > buffer->cap) {
            buffer->pop = 0;
        }
    }
}
// I feel like there might be a simplified way but who cares.
static size_t sArraySize(SArray * buffer) {
    size_t size = 0;
    if (buffer->push < buffer->pop) {
        // size to push index + size from pop index to end of buffer , 
        //  +1 because push index starts at zero, not +2 because cap and how numbers work.
        size = buffer->push + (buffer->cap - buffer->pop) + 1;
    } else if (buffer->push > buffer->pop) {
        size = buffer->push - buffer->pop;
    } // else equals so return 0
    return size; 
}

// now we implement heap insert and remove, iterations?

    // yeah, need to think about this...
    //
    //  let's define a left and right node for each node.
    //      let's also maintain that current < left < right node. Necessary?

    //  pop from root, 
    //  push from root,
    //  

    // 

// hmmm... seems dumb but let's roll with it.

// TODO: fix ret values to be something meaningful
static int memPush(TreeNode * node, TreeNode * newNode) {
    // traverse heap until satisfy min heap property. and left < right
    if (node == NULL) { // don't be an idiot
        return 0;
    }
    MemNode * newMemNode = (MemNode *)newNode->data_ptr;
    // because min heap, 
    // if current_node > size, then 
    //  wtf even is a binary heap? 
    // hmmm... for now let's swap node with NewNode and make node child of newNode.
    // TODO: how to binary tree? because this will get unbalanced quickly?
    //      it's 
    MemNode * memNode = (MemNode *)node->data_ptr;
    if (memNode->block_size > newMemNode->block_size) {
        // TODO: enforce left < right, and maybe go left, right, left children? prioritize ehh probably not but interesting thought.
        newNode->left = node;
        newNode->parent = node->parent;
        if (node->parent->left == node) {
            node->parent->left = newNode;
        } else {
            node->parent->right = newNode;
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
        int ret = memPush(node->left, newNode);
        if (ret == 1) { // means we found a place for this damn thing....
            return ret;
        }
    }
    if (node->right != NULL) {
        return memPush(node->right, newNode);
    }
}

static TreeNode * memPop(TreeNode * node, size_t size) {
    if (node == NULL) { // don't be an idiot
        return 0;
    }
    // traverse heap until size <= mem_node.block_size,
    MemNode * mem_node = (MemNode *)current_node->data_ptr;
    // So, I think for this, since we are implementing a binary heap, we can assume, current_node < left < right?
    //  realistically we can grab any node as long as it satifies that request, but minimizing size minimizes fragmentation? 
    //      this might be too much lol

    // BIG O(log n) but since?
    if (size <= mem_node->block_size) {
        // if node satisfies condition of >size

        // TODO: actually remove the node.... LMAO

        return node;
    } 

    // else traverse tree
    if (node->left == NULL && node->right == NULL) {
        return NULL;
    }
    TreeNode * ret_node = NULL;
    if (node->left != NULL) {
        ret_node = memPop(node->left);
        if (ret_node != NULL) {
            return ret_node;
        }
    }
    if (node->right != NULL) {
        return memPop(node->right);
    }
}

#define DYNAMIC_MEMORY_SIZE 2 << 32;
// void arr[] valid? lol (void == size_t?)
static void DYNAMIC_MEMORIES[DYNAMIC_MEMORY_SIZE] = {};

// TODO:
// What do we do if, run out of blocks? 
//  if we make this size of individial indicies (DYNAMIC_MEMORY_SIZE), then we will never reach this limit?
#define MAX_MEM_NODES DYNAMIC_MEMORY_SIZE;
static TreeNode[MAX_MEM_NODES] = {}; 
static MemNode[MAX_MEM_NODES] = {}; 

static SArray treeArray;
sArrayInit(&treeArray, MAX_MEM_NODES);
static SArray memArray;
sArrayInit(&memArray, MAX_MEM_NODES);

// initialize root node
sArrayAdd(&memArray);
memArray[memArray.push] = { .ptr = DYNAMIC_MEMORIES, .block_size = DYNAMIC_MEMORY_SIZE };

// tree array needs to be split to used and free...
sArrayAdd(&treeArray);
treeArray[treeArray.push] = { .data_ptr = memArray[memArray.push], .parent = NULL, .left = NULL, .right = NULL };

TreeNode * rootNode = treeArray[treeArray.push];
// root node is parent == NULL;
//  because memoization (without it, you basically perform the following everytime you need that information?)

// yeah, thinking about this some more might seldom use this but good to have? 
//  without any paging, this is fucking dumb :)
extern void * emalloc(size_t size) {
    // hmmm... yeah so 
    TreeNode * node = memPop(rootNode, size);

    // TODO:
    // input validation?

    // TODO:
    // get address from node, insert update node (only if block_size > size, else we remove...) in free structure...
    // TODO: might specialize the structure and remove the need for this 
    MemNode * memNode = (MemNode *)node->data_ptr;

    if (node == NULL || node->block_size < size) { 
        // freak out 
        return NULL;
    } 
    
    void * extracted_ptr = memNode->ptr;
    if (node->block_size == size) {
        // just move the block to the used list
        // LMAO
        memPush(usedRootNode, node);
    } else { // if (node->block_size > size) {
        memNode->ptr += size;
        memNode->block_size -= size;
 
        // re-insert updated node (in freed list), with new size...
        memPush(rootNode, node);

        // create new node to add to used array...
        //      since there are finite number of addresses... our node buffers can be set to size of memory array and we'll never go past the limit.
        //      TODO: is this an acceptable tradeoff... hmm... actually might apply to the array implementation too... derp.

        // more abstraction

        // create new used memory node.
        if (1 == sArrayAdd(&memArray)) {
            // TODO:
            // What do we do if, run out of blocks? 
            //  if we make this size of individial indicies (DYNAMIC_MEMORY_SIZE), then we will never reach this limit?
        }
        memArray[memArray.push] = { .ptr = extracted_ptr, .block_size = size };
  
        //  create new used tree node...
        if (1 == sArrayAdd(&treeArray)) {
            // TODO:
            // What do we do if, run out of blocks? 
            //  if we make this size of individial indicies (DYNAMIC_MEMORY_SIZE), then we will never reach this limit?
        }
        treeArray[treeArray.push] = { .data_ptr = memArray[memArray.push],.parent = NULL, .left = NULL, .right = NULL };
  
        if (1 == memPush(rootNode, treeArray[treeArray.push])) {
            // TODO: this will change when ret values fixed as described above...
        } // else error?
    }
}

extern void efree(void * ptr);

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