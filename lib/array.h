#ifndef ARRAY_H
#define ARRAY_H

#if defined __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include "result.h"

#define ARRAY_RECOMMENDED_INITIAL_CAP 8

typedef struct Array {
    void * buf;
    size_t size; // byte count
    size_t cap;
    size_t size_of_el;
} Array;

// TODO: then try implementing this with CPP classes, generic types etc? to see the difference?
extern Array * array_constructor(const size_t initial_cap, const size_t size_of_el);
extern void array_initialize(Array * arr, void * buf, const size_t initial_cap, const size_t size_of_el);
extern void array_destructor(Array * arr);

extern operation_result array_append(Array * arr, const void * els, const size_t num_els);
extern operation_result array_insert(Array * arr, const size_t pos, const void * els, const size_t num_els);
extern operation_result array_remove(Array * arr, const size_t pos, const size_t num_els);

// size/size_of_el
extern size_t array_num_elements(Array * arr);

#if defined __cplusplus
}
#endif

#endif 