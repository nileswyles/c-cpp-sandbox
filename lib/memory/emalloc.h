#ifndef EMALLOC_H
#define EMALLOC_H

#include <stdbool.h>

// right?
#define malloc emalloc

#define DYNAMIC_MEMORY_SIZE 2 << 32;

extern void * emalloc(size_t size);
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