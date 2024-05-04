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
static constexpr double RESIZE_FACTOR = 1.75;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

// Template Class
template<typename T>
class Array {
    private:
        size_t cap;
        // TODO:
        //  since creating new buffer everytime, let's just create size needed.
        //  memory allocations are cheap apparently
        T * recapBuffer(size_t num_els) {
            printf("num_els: %d, size: %d, cap: %d\n", num_els, this->size, this->cap);
            T * new_buf = nullptr; 
            if (num_els + this->size > this->cap) {
                size_t new_cap = (size_t)((num_els + this->size) * RESIZE_FACTOR);
                new_buf = newCArray<T>(new_cap);
                if (new_buf != NULL) {
                    this->cap = new_cap;
                }
            } else {
                new_buf = newCArray<T>(this->cap);
            }
            return new_buf;
        }
        size_t addElements(T * buf, const size_t pos, const T * els, const size_t num_els) {
            for (size_t i = 0; i < num_els; i++) {
                buf[i + pos] = els[i];
                printf("%x, %x, %x, %x\n", buf[i + pos], els[i], &buf[i + pos], &els[i]);
            }
            return num_els * sizeof(T);
        }
    public:
        T * buf;
        size_t size;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<T>(initial_cap)) {}
        // ~Array() {
        //     delete[] this->buf;
        // }
        operation_result append(const T * els, const size_t num_els) {
            return this->insert(this->size, els, num_els);
        }
        operation_result insert(const size_t pos, const T * els, const size_t num_els) {
            // O(n) with at least one alloc
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            T * new_buf = this->recapBuffer(num_els);
            size_t total_size = this->size * sizeof(T);
            if (new_buf == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }

            size_t total_size_up_to_pos = pos * sizeof(T);
            memcpy((void *)new_buf, (void *)this->buf, total_size_up_to_pos);
            size_t total_size_new_els = this->addElements(new_buf, pos, els, num_els);
            size_t total_size_after_pos = (this->size - pos) * sizeof(T);
            memcpy(((void *)new_buf) + total_size_up_to_pos + total_size_new_els, ((void *)this->buf) + total_size_up_to_pos, total_size_after_pos);

            // This should only remove pointers to strings, not strings, no?
            delete[] this->buf;
            this->buf = new_buf;

            this->size += num_els;

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // O(n) with at least one alloc
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            T * new_buf = this->recapBuffer(num_els);
            size_t total_size = this->size * sizeof(T);
            if (new_buf == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t total_size_up_to_pos = pos * sizeof(T);
            memcpy((void *)new_buf, (void *)this->buf, total_size_up_to_pos); // copy from buf up to pos
            size_t total_size_to_remove = num_els * sizeof(T);
            size_t total_size_after_removed = (this->size - pos - num_els) * sizeof(T);
            memcpy(((void *)new_buf) + total_size_up_to_pos, ((void *)this->buf) + total_size_up_to_pos + total_size_to_remove, total_size_after_removed);

            // This should only remove pointers to strings, not strings, no?
            delete[] this->buf;
            this->buf = new_buf;

            this->size += num_els;

            return OPERATION_SUCCESS;
        }
        std::string toString() {
            uint8_t nul[1] = {0};
            this->append(nul, 1);
            return std::string((char *)this->buf);
        }
        T operator [] (const size_t pos) {
            return this->buf[pos];
        }
};

// The reason for this is to create copys of the strings pointed to by els so that we don't have dangling pointers to strings that no longer exist. 
//  In other words, the memory of those strings are managed by this class. This should really apply for all pointer types, but I'm not sure that's even possible so let's start with this for now.

//  After alot of toil for nothing! Template class specialization needed because destructor... Need to revisit this in the future. Can you function template the destructor?
template<>
class Array<char*> {
    private:
        size_t cap;
        // TODO:
        //  since creating new buffer everytime, let's just create size needed.
        //  memory allocations are cheap apparently
        char ** recapBuffer(size_t num_els) {
            printf("num_els: %d, size: %d, cap: %d\n", num_els, this->size, this->cap);
            char ** new_buf = nullptr; 
            if (num_els + this->size > this->cap) {
                size_t new_cap = (size_t)((num_els + this->size) * RESIZE_FACTOR);
                new_buf = newCArray<char *>(new_cap);
                if (new_buf != NULL) {
                    this->cap = new_cap;
                }
            } else {
                new_buf = newCArray<char *>(this->cap);
            }
            return new_buf;
        }
        size_t addElements(char ** buf, const size_t pos, char ** els, const size_t num_els) {
            // Iterate over char pointers (els) - for each:
            //          allocate strlen.
            //          copy cstring at els[i] to new pointer location
            //          store new pointer at this->buf[i];

            //  Pointers will be contiguous in memory but cstring data might not... Is this an issue?
            //      not sure how that would even be possible but does it have to be something like:
            //      1: [1,2,3,4,5] 
            //      6: [6,7,8,9,10]
            //      instead of:
            //      1: [1,2,3,4,5] 
            //      27: [27,28,29,30,31]
            for (size_t i = 0; i < num_els; i++) {
                // start at pos
                size_t buf_i = i + pos;
                // allocate memory for new cstring
                char * new_cstring = newCArray<char>(strlen(els[i]) + 1);
                // copy els[i] into new_cstring
                strcpy(new_cstring, els[i]);
                printf("New String: %s\n", new_cstring);
                // store new string pointer
                buf[buf_i] = new_cstring;
                printf("pointer to first character of string in buffer location %x\n", this->buf[buf_i]);
            }
            return num_els * sizeof(char *);
        }
    public:
        char ** buf;
        size_t size;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<char *>(initial_cap)) {}
        ~Array() {
            // See char * overloaded version of addElements
            for (size_t i = 0; i < size; i++) {
                delete[] this->buf[i];
            }
            delete[] this->buf;
        }
        operation_result append(char ** els, const size_t num_els) {
            return this->insert(this->size, els, num_els);
        }
        operation_result insert(const size_t pos, char ** els, const size_t num_els) {
            // O(n) with at least one alloc
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            char ** new_buf = this->recapBuffer(num_els);
            size_t total_size = this->size * sizeof(char *);
            if (new_buf == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }

            size_t total_size_up_to_pos = pos * sizeof(char *);
            memcpy((void *)new_buf, (void *)this->buf, total_size_up_to_pos);
            size_t total_size_new_els = this->addElements(new_buf, pos, els, num_els);
            size_t total_size_after_pos = (this->size - pos) * sizeof(char *);
            memcpy(((void *)new_buf) + total_size_up_to_pos + total_size_new_els, ((void *)this->buf) + total_size_up_to_pos, total_size_after_pos);

            // This should only remove pointers to strings, not strings, no?
            delete[] this->buf;
            this->buf = new_buf;

            this->size += num_els;

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // O(n) with at least one alloc
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            char ** new_buf = this->recapBuffer(num_els);
            size_t total_size = this->size * sizeof(char *);
            if (new_buf == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t total_size_up_to_pos = pos * sizeof(char *);
            memcpy((void *)new_buf, (void *)this->buf, total_size_up_to_pos); // copy from buf up to pos
            size_t total_size_to_remove = num_els * sizeof(char *);
            size_t total_size_after_removed = (this->size - pos - num_els) * sizeof(char *);
            memcpy(((void *)new_buf) + total_size_up_to_pos, ((void *)this->buf) + total_size_up_to_pos + total_size_to_remove, total_size_after_removed);

            // This should only remove pointers to strings, not strings, no?
            delete[] this->buf;
            this->buf = new_buf;

            this->size += num_els;

            return OPERATION_SUCCESS;
        }
        std::string toString() {
            char * nul[1] = {0};
            this->append(nul, 1);
            return std::string((char *)this->buf);
        }
        char * operator [] (const size_t pos) {
            return this->buf[pos];
        }
};

// TODO: read reference sections on templates lol
// And only because function specialization not working? because then it's really just specializing addElements and destructor
//  might be worth keeping as an example, for future reference?

// Template Class Member functions

// TODO: remove excessive allocations and dependency on memcpy
    // also void return types and handle exceptions?

// TODO:
//  2D arrays, like character strings aren't copied.

//  I think what's happening is it's copying the first character of each string...
//      In other words, the array has a pointer to start of each string, but underlying data isn't copied?
//      Or something to that effect... let's see.

//  Alright, so if T == char *, sizeof = strlen(els), that's it?
//      and usage of els is char * not char ** like what I'm doing now...
//    actually, no.... but yes, if T == char * then els type == char ** and sizeof += strlen(els[i])

//    or flatten strings? and "split" on nul, somehow?

//  or allocate "heap" memory for strings seperately? last last last last last resort

// Here's the thing, it's definetly doable because vector lol
}
#endif 