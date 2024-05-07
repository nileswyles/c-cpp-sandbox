#ifndef EMALLOC_H
#define EMALLOC_H

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
            //   malloc will iterate over freed blocks, find min address where size > new_size;
            //   update freed block and add used block, 

            //   freed will find used block at address provided... 
            //      then remove entry from used blocks list completly and add new freed entry...

            //   and so, no need to iterate more than length of lists described above.

            //   so not really a heap lol just a list of used and freed blocks...
            //   tradeoffs, heap insert actually more than worth for this problem?
            //
            //   hmm... seems there's a lot more to go... (first let's confirm the solution described above actually works.)
            //      practice makes better? but not best?

            //      what's faster than BIG O(n)? 
            //      O(log n) and O(1)

            //      log's typically mean we split search space in half periodically...
            //          
            //      O(1), wtf even is that? Hashes?

///     because memoization (without it, you basically perform the following everytime you need that information?)
extern void * emalloc(size_t size);

extern void efree(void * ptr);

#endif