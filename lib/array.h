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

template<typename T>
void deleteCArray(T * buf, size_t size) {
    delete[] buf;
}

template<>
void deleteCArray<char *>(char ** buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        delete[] buf[i];
    }
    delete[] buf;
}

// This will create a class for each combination of Array-subtype instantiated... in the case of the tests, we'll get class definitions for Array<uint8_t> and Array<char *>...
//  which is good!
//  now if only I can specialize functions (implement functions specific to (sub)type, to be clear), instead of the entire class... then we would be templating for real.

// gahhhhh, why doesn't this just work!

// Template Class
template<typename T>
class Array {
    private:
        // template<typename T>
        void addElement(T * buf, const size_t pos, T el) {
            buf[pos] = el;
        }
        // The reason for this is to create copys of the strings pointed to by els so that we don't have dangling pointers to strings that no longer exist. 
        //  In other words, the memory of those strings are managed by this class. This should really apply for all pointer types, but I'm not sure that's even possible so let's start with this for now.
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

        //  After alot of toil for nothing! Template class specialization needed because destructor... Need to revisit this in the future. Can you function template the destructor?
        // template<>
        void addElement(char ** buffer, const size_t pos, char * el) {
            // allocate memory for new cstring
            char * new_cstring = newCArray<char>(strlen(el) + 1);
            // copy el into new_cstring
            strcpy(new_cstring, el);
            printf("New String: %s\n", new_cstring);
            // store new string pointer
            buffer[pos] = new_cstring;
            printf("pointer to first character of string in buffer location %x\n", buffer[pos]);
        }
        size_t cap;
    public:
        T * buf;
        size_t size;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<T>(initial_cap)) {}
        ~Array() {
            deleteCArray<T>(this->buf, this->size);
        }
        operation_result append(const T * els, const size_t num_els) {
            return this->insert(this->size, els, num_els);
        }
        operation_result insert(const size_t pos, const T * els, const size_t num_els) {
            // O(n) with at least one alloc
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            printf("num_els: %ld, size: %ld, cap: %ld\n", num_els, this->size, this->cap);

            T * new_buf = this->buf; 
            // recap buffer if needed. 
            if (num_els + this->size > this->cap) {
                size_t new_cap = (size_t)((num_els + this->size) * RESIZE_FACTOR);
                new_buf = newCArray<T>(new_cap);
                if (new_buf == nullptr) {
                    return MEMORY_OPERATION_ERROR;
                } else {
                    this->cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically intialized by insert operation... (see use new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(T);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = this->buf[i];
                    }
                }
            }
            // new insert/append logic
            //  create array of size num_els. 
            //  iterate over positions to replace, if at > start pos, throw in array bucket and set corresponding value at els
            //  once done inserting values, we'll grab oldest value in array...
            T * temp_buff = newCArray<T>(num_els);
            if (temp_buff == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            // make shift stack
            size_t temp_push = 0;
            size_t temp_pop = 0;
            // num_els == 2
            // els = [1, 2]
            // buf_pos = [6, 7, 8, 9, 10]
                        //     push, pop
            // temp_buff = [] ; 0, 0        ; buf = [6, 7, 8, 9, 10]
            // temp_buff = [6] ; 1, 0       ; buf = [1, 7, 8, 9, 10]
            // temp_buff = [6, 7] ; 2, 0    ; buf = [1, 2, 8, 9, 10]
            // temp_buff = [8, 7] ; 0, 1    ; buf = [1, 2, 6, 7]
            for (size_t i = pos; i < this->size + num_els; i++) {
                T value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                    // call addElement with els value
                } else {
                    // make sure to pop before push, else you need num_els + 1?
                    value = temp_buff[temp_pop];
                    if (++temp_pop == num_els) {
                        // increment temp index and wrap if needed...
                        temp_pop = 0;
                    }
                    // call addElement with oldest value in temp_buff
                }
                temp_buff[temp_push] = this->buf[i]; // store value
                if (++temp_push == num_els) {
                    // increment temp index and wrap if needed...
                    temp_push = 0;
                }
                addElement(new_buf, i, value);
            }
            delete[] temp_buff;

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

            for (size_t i = pos + num_els; i < this->size; i++) {
                this->buf[i - num_els] = this->buf[i];
            }
            this->size -= num_els;

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
}
#endif 