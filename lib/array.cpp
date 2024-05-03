#include "array.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

using namespace WylesLibs;

// TODO: remove excessive allocations and dependency on memcpy
    // also void return types and handle exceptions?
static inline size_t resizeBuffer(size_t num_els, size_t current_size);
static constexpr double RESIZE_FACTOR = 1.75;

template <class T>
operation_result Array<T>::append(const T * els, const size_t num_els) {
    // O(n) with at least no alloc, maybe one alloc

    size_t total_size = this->size * sizeof(T);
    if (num_els + this->size > this->cap) {
        T * new_buf = newCArray<T>(resizeBuffer(num_els, this->size));
        if (new_buf == NULL) {
            return MEMORY_OPERATION_ERROR;
        }
        memcpy((void *)new_buf, (void *)this->buf, total_size);
        delete this->buf;
        this->buf = new_buf;
    } 

    size_t total_size_new_els = num_els * sizeof(T);
    memcpy(this->buf + total_size, els, total_size_new_els);

    this->size += num_els;

    return OPERATION_SUCCESS;
}

template <class T>
operation_result Array<T>::insert(const size_t pos, const T * els, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > this->size) return OPERATION_ERROR;

    T * new_buf = newCArray<T>(resizeBuffer(num_els, this->size));
    if (new_buf == NULL) {
        return MEMORY_OPERATION_ERROR;
    }
    // removed const from els,,, if not resizing the entire buf,
    // start from pos, swap els from els and buf until reach num els, then append the swapped bytes in els...
    // resulting in no allocation... in this->case, data is already allocated (in stack?) from calling function? 

    // something to consider, does the compiler make some sort of optimization when stack data is const that makes what I'm proposing a bad idea?? 

    size_t total_size_up_to_pos = pos * sizeof(T);
    memcpy(new_buf, this->buf, total_size_up_to_pos); // copy from buf up to pos
    size_t total_size_new_els = num_els * sizeof(T);
    memcpy(new_buf + total_size_up_to_pos, els, total_size_new_els);
    size_t total_size_after_pos = (this->size - pos) * sizeof(T);
    memcpy(new_buf + total_size_up_to_pos + total_size_new_els, this->buf + total_size_up_to_pos, total_size_after_pos);

    delete this->buf;
    this->buf = new_buf;

    return OPERATION_SUCCESS;
}

template <class T>
operation_result Array<T>::remove(const size_t pos, const size_t num_els) {
    // O(n) with at least one alloc
    // pos out of bounds, return error...
    if (pos < 0 || pos > this->size) return OPERATION_ERROR;

    T * new_buf = newCArray<T>(this->size);
    if (new_buf == NULL) {
        return MEMORY_OPERATION_ERROR;
    }

    // can do this->without re-allocating the entire buffer? but is it worth making that optimization?
    // more and more worth it for pos closer to size, do we have to deal with stack size constraints?

    size_t total_size_up_to_pos = pos * sizeof(T);
    memcpy(new_buf, this->buf, total_size_up_to_pos); // copy from buf up to pos

    size_t total_size_to_remove = num_els * sizeof(T);
    size_t total_size_after_removed = (this->size - pos - num_els) * sizeof(T);
    memcpy(new_buf + total_size_up_to_pos, this->buf + total_size_up_to_pos + total_size_to_remove, total_size_after_removed);

    delete this->buf;
    this->buf = new_buf;

    return OPERATION_SUCCESS;
}

static inline size_t resizeBuffer(size_t num_els, size_t current_size) {
    return (size_t)((num_els + current_size) * RESIZE_FACTOR);
}