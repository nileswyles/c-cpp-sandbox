#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include "result.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define ARRAY_RECOMMENDED_INITIAL_CAP 8

namespace WylesLibs {

template <class T>
static inline T * newCArray(size_t size) {
    return (T *) ::operator new (sizeof(T) * size);
}

// hm...... so templates in header file only, makes sense? array translation unit (array.cpp) didn't have any source... 
    // I wonder what that means for code size, I always thought using the linker helped with that? like linking translation units together?
// and only header stuff is prepended to translation units using #include, right? How to get a definitive answer
//      linker doesn't have anything to link lol, so I guess that's the answer :)

//      if it works - ship it!

//   but no cleaner way to implement? .h makes easier to import - in any case. 
//        maybe move template definition to class definition? or do template member functions need to be declared and implement/defined separately?

// Template Class

template <class T>
class Array {
    private:
        size_t cap;
    public:
        T * buf;
        size_t size;
        // TODO: Can uniform initialize the array data?
        //      Begin and end to support range-based loops?
        //          i.e. for (auto e: obj){}
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<T>(initial_cap)) {
            // this.buf = newCArray<T>(initial_cap);
            // this.size = 0;
        }
        ~Array() {
            delete buf;
        }
        operation_result append(const T * els, const size_t num_els);
        // NOT TO BE CONFUSED WITH VECTOR INSERT LOL
        operation_result insert(const size_t pos, const T * els, const size_t num_els);
        operation_result remove(const size_t pos, const size_t num_els);

        // could potentially use class specialization but lol cmon
        std::string toString() {
            uint8_t nul[1] = {0};
            this->append(nul, 1);
            return std::string((char *)this->buf);
        }
        //T operator [] (const size_t& pos) {
        T * operator [] (const size_t pos) {
            // TODO: Update value at position...
            // can I implement something like arr[i] = value? To update value at position...
            // pointer arithmetic a sin?
            return buf + pos;
        }
};

// Template Class Member functions

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

}
#endif 