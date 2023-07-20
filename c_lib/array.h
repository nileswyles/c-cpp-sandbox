#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#define ARRAY_RECOMMENDED_INITIAL_CAP 8

typedef struct Array {
    void * buf;
    size_t size;
    size_t cap;
    size_t size_of_el;
} Array;


// TODO: then try implementing this with CPP classes, generic types etc? to see the difference?
extern Array * array_constructor(const size_t initial_cap, const size_t size_of_el);
extern void array_destructor(Array * arr);

extern void array_append(Array * arr, void * els, const size_t num_els);
extern void array_insert(Array * arr, const size_t pos, void * els, const size_t num_els);
extern void array_remove(Array * arr, const size_t pos, const size_t num_els);

#endif 