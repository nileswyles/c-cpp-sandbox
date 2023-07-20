#include "array.h"

static inline void * alloc_new_buf(Array * arr);
static inline void * resize_buf(Array * arr, const size_t num_els);

extern Array * array_constructor(const size_t initial_cap, const size_t size_of_el) {
    Array * arr = (Array *)malloc(sizeof(Array));
    arr->cap = initial_cap; 
    arr->size_of_el = size_of_el;
    arr->size = 0;
    
    arr->buf = alloc_new_buf(arr);

    return arr;
}

extern void array_destructor(Array * arr) {
    free(arr->buf);
    free(arr);
}

extern void array_append(Array * arr, const void * els, const size_t num_els) {
    // O(n) with at least no alloc, maybe one alloc
    void * new_buf = resize_buf(arr, num_els);

    size_t total_size = arr->size * arr->size_of_el;
    if (new_buf != NULL) {
        memcpy(new_buf, arr->buf, total_size);
        free(arr->buf);
        arr->buf = new_buf;
    } 

    size_t total_size_new_els = num_els * arr->size_of_el;
    memcpy(arr->buf + total_size, els, total_size_new_els);
}

extern void array_insert(Array * arr, const size_t pos, const void * els, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > arr->size) return NULL;

    void * new_buf = resize_buf(arr, num_els);
    if (new_buf == NULL) {
        new_buf = alloc_new_buf(arr);
    }

    size_t total_size_up_to_pos = pos * arr->size_of_el;
    memcpy(new_buf, arr->buf, total_size_up_to_pos); // copy from buf up to pos
    size_t total_size_new_els = num_els * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos, els, total_size_new_els);
    size_t total_size_after_pos = (arr->size - pos) * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos + total_size_new_els, arr->buf + total_size_up_to_pos, total_size_after_pos);

    free(arr->buf);
    arr->buf = new_buf;
}

extern void array_remove(Array * arr, const size_t pos, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > arr->size) return NULL;

    void * new_buf = alloc_new_buf(arr);

    size_t total_size_up_to_pos = pos * arr->size_of_el;
    memcpy(new_buf, arr->buf, total_size_up_to_pos); // copy from buf up to pos

    size_t total_size_to_remove = num_els * arr->size_of_el;
    size_t total_size_after_removed = (arr->size - pos - num_els) * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos, arr->buf + total_size_up_to_pos + total_size_to_remove, total_size_after_removed);

    free(arr->buf);
    arr->buf = new_buf;
}

static inline void * alloc_new_buf(Array * arr) {
    // TODO: NULL check...
    return malloc(arr->cap*arr->size_of_el);
}

static inline void * resize_buf(Array * arr, const size_t num_els) {
    void * new_buf = NULL;
    if (num_els + arr->size > arr->cap) {
        // TODO: hmm.....
        size_t new_cap = (arr->cap * 2);
        arr->cap = new_cap;
        new_buf = alloc_new_buf(arr);
    } 
    return new_buf;
}