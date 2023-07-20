#include "array.h"

#include <stdbool.h>

static inline void * alloc_new_buf(Array * arr);
static inline bool resize_buf(Array * arr, const size_t num_els);

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

extern operation_result array_append(Array * arr, const void * els, const size_t num_els) {
    // O(n) with at least no alloc, maybe one alloc

    size_t total_size = arr->size * arr->size_of_el;
    if (resize_buf(arr, num_els)) {
        void * new_buf = alloc_new_buf(arr);
        if (new_buf == NULL) {
            return MEMORY_OPERATION_ERROR;
        }
        memcpy(new_buf, arr->buf, total_size);
        free(arr->buf);
        arr->buf = new_buf;
    } 

    size_t total_size_new_els = num_els * arr->size_of_el;
    memcpy(arr->buf + total_size, els, total_size_new_els);

    return OPERATION_SUCCESS;
}

extern operation_result array_insert(Array * arr, const size_t pos, void * els, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > arr->size) return NULL;

    resize_buf(arr, num_els);
    void * new_buf = alloc_new_buf(arr);
    if (new_buf == NULL) {
        return MEMORY_OPERATION_ERROR;
    }

    // removed const from els,,, if not resizing the entire buf,
    // start from pos, swap els from els and buf until reach num els, then append the swapped bytes in els...
    // resulting in no allocation... in this case, data is already allocated (in stack?) from calling function? 

    size_t total_size_up_to_pos = pos * arr->size_of_el;
    memcpy(new_buf, arr->buf, total_size_up_to_pos); // copy from buf up to pos
    size_t total_size_new_els = num_els * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos, els, total_size_new_els);
    size_t total_size_after_pos = (arr->size - pos) * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos + total_size_new_els, arr->buf + total_size_up_to_pos, total_size_after_pos);

    free(arr->buf);
    arr->buf = new_buf;

    return OPERATION_SUCCESS;
}

extern operation_result array_remove(Array * arr, const size_t pos, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > arr->size) return NULL;

    void * new_buf = alloc_new_buf(arr);
    if (new_buf == NULL) {
        return MEMORY_OPERATION_ERROR;
    }

    // can do this without re-allocating the entire buffer? but is it worth making that optimization?
    // more and more worth it for pos closer to size, do we have to deal with stack size constraints?

    size_t total_size_up_to_pos = pos * arr->size_of_el;
    memcpy(new_buf, arr->buf, total_size_up_to_pos); // copy from buf up to pos

    size_t total_size_to_remove = num_els * arr->size_of_el;
    size_t total_size_after_removed = (arr->size - pos - num_els) * arr->size_of_el;
    memcpy(new_buf + total_size_up_to_pos, arr->buf + total_size_up_to_pos + total_size_to_remove, total_size_after_removed);

    free(arr->buf);
    arr->buf = new_buf;

    return OPERATION_SUCCESS;
}

static inline void * alloc_new_buf(Array * arr) {
    return malloc(arr->cap*arr->size_of_el);
}

static inline bool resize_buf(Array * arr, const size_t num_els) {
    bool resized = false;
    if (num_els + arr->size > arr->cap) {
        // TODO: hmm.....
        size_t new_cap = (arr->cap * 2);
        arr->cap = new_cap;
        resized = true;
    } 
    return resized;
}