#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "result.h"
#include "logger.h"

namespace WylesLibs {

constexpr size_t ARRAY_RECOMMENDED_INITIAL_CAP = 8;
constexpr double RESIZE_FACTOR = 1.75;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

// TODO: exceptions...
template<typename T>
class Array {
    private:
        size_t e_cap;
        size_t e_size;
        void addElement(T * buf, const size_t pos, T el) {
            buf[pos] = el;
        }
    public:
        // hmm... need to think about this again... 
        T * buf;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : e_cap(initial_cap), e_size(0), buf(newCArray<T>(initial_cap)) {}
        ~Array() {
            delete[] buf;
        }
        size_t size() {
            return this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return this->e_cap;
        }
        operation_result uniqueAppend(T& el) {
            if (this->contains(el)) { 
                return OPERATION_ERROR;
            } else {
                return this->append(&el, 1);
            }
        }
        operation_result append(T& el) {
            return this->append(&el, 1);
        }
        operation_result append(const T * els, const size_t num_els) {
            return this->insert(this->e_size, els, num_els);
        }
        operation_result insert(const size_t pos, const T& el) {
            return this->insert(pos, el, 1);
        }
        operation_result insert(const size_t pos, const T * els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->e_size) return OPERATION_ERROR;

            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->e_size, this->e_cap, pos);

            T * new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->e_size > this->e_cap) {
                size_t new_cap = (size_t)((num_els + this->e_size) * RESIZE_FACTOR);
                new_buf = newCArray<T>(new_cap);
                if (new_buf == nullptr) {
                    return MEMORY_OPERATION_ERROR;
                } else {
                    recapped = true;
                    this->e_cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically intialized by insert operation... (see use of new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(T);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = this->buf[i];
                    }
                }
            }
            
            T * bucket = newCArray<T>(num_els);
            if (bucket == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t bucket_push = 0;
            size_t bucket_pop = 0;
            for (size_t i = pos; i < this->e_size + num_els; i++) {
                T value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                } else {
                    value = bucket[bucket_pop];
                    if (++bucket_pop == num_els) {
                        bucket_pop = 0;
                    }
                }
                if (i < this->e_size) {
                    bucket[bucket_push] = this->buf[i];
                    if (++bucket_push == num_els) {
                        bucket_push = 0;
                    }
                }
                addElement(new_buf, i, value);
            }
            delete[] bucket;

            if (recapped) {
                delete[] this->buf;
                this->buf = new_buf;
            }
            this->e_size += num_els;

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos >= this->e_size) return OPERATION_ERROR;

            for (size_t i = pos + num_els; i < this->e_size; i++) {
                this->buf[i - num_els] = this->buf[i];
            }
            this->e_size -= num_els;

            return OPERATION_SUCCESS;
        }
        bool contains(const T& el) {
            return find(el) != -1;
        }
        size_t find(const T& el) {
            for (size_t i = 0; i < this->e_size; i++) {
                if (this->buf[i] == el) {
                    return i;
                }
            }
            return -1;
        }
        T& at(const size_t pos) {
            if (pos > 0 && pos < this->e_size) {
                return this->buf[pos];
            } else {
                // throw std::runtime_error("Invalid pos...");
            }
        }
        T& front() {
            if (this->e_size == 0) {
                T nul = {0};
                this->append(nul);
            }
            return this->buf[0];
        }
        T& back() {
            if (this->e_size == 0) {
                T nul = {0};
                this->append(nul);
            }
            return this->buf[this->e_size-1];
        }
        Array& popBack() {
            remove(this->e_size-1);
            return *this; // lmaooo...
        }
        std::string toString() {
            T nul = {0};
            this->append(nul);
            return std::string((char *)this->buf);
        }
        T& operator [] (const size_t pos) {
            return this->buf[pos];
        }
        T& operator [] (const T& el) {
            size_t i = this->find(el);
            if (i == -1) {
                this->append(el); 
                this->buf[this->e_size-1];
            } else {
                return this->buf[i];
            }
        }
        // copy... 
        // Array(const T& x) : e_cap(x.cap()), size(x.getSize()), buf(x.buf) {}
        // T& operator= (const T& x) {
        //     this->buf = x.buf;
        //     // lol.. this doesn't seem like the right way to do this?
        //     //  let's find out...
        //     this->e_cap = x.cap();
        //     this->size = x.getSize();

        //     // how do containers that don't expose pointer to underlying data do this?
        //     //  shouldn't this just be the default?
        //     //  why do I even need to implement this?

        //     //  hmm... my example of incrementing some id value doesn't really make sense either lol. so wtf is this?
        //     return *this;
        // }
        // Move
        // Array(T&& x) : e_cap(x.cap()), size(x.getSize()), buf(x.buf) {}
        // T& operator= (T&& x) {
        //     this->buf = x.buf; // lol.. this seems like not the right way to do this?
        //     //  let's find out...
        //     this->e_cap = x.cap();
        //     this->size = x.getSize();
        //     return *this;
        // }
};

template<>
class Array<const char *> {
    private:
        size_t e_cap;
        size_t e_size;
        void addElement(char ** buffer, const size_t pos, const char * el) {
            char * new_cstring = newCArray<char>(strlen(el) + 1);
            strcpy(new_cstring, el);
            loggerPrintf(LOGGER_DEBUG, "New String: %s\n", new_cstring);
            buffer[pos] = new_cstring;
        }
    public:
        char ** buf;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : e_cap(initial_cap), e_size(0), buf(newCArray<char *>(initial_cap)) {}
        ~Array() {
            for (size_t i = 0; i < this->e_size; i++) {
                delete[] buf[i];
            }
            delete[] buf;
        }
        size_t size() {
            return this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return this->e_cap;
        }
        operation_result append(const char * el) {
            return this->append(&el, 1);
        }
        operation_result append(const char ** els, const size_t num_els) {
            return this->insert(this->e_size, els, num_els);
        }
        operation_result insert(const size_t pos, const char * el) {
            return this->insert(pos, &el, 1);
        }
        operation_result insert(const size_t pos, const char ** els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->e_size) return OPERATION_ERROR;

            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->e_size, this->e_cap, pos);

            char ** new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->e_size > this->e_cap) {
                size_t new_cap = (size_t)((num_els + this->e_size) * RESIZE_FACTOR);
                new_buf = newCArray<char *>(new_cap);
                if (new_buf == nullptr) {
                    return MEMORY_OPERATION_ERROR;
                } else {
                    recapped = true;
                    this->e_cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically intialized by insert operation... (see use of new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(char *);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = this->buf[i];
                    }
                }
            }

            char ** bucket = newCArray<char *>(num_els);
            if (bucket == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t bucket_push = 0;
            size_t bucket_pop = 0;
            for (size_t i = pos; i < this->e_size + num_els; i++) {
                loggerPrintf(LOGGER_DEBUG, "push: %ld pop: %ld\n", bucket_push, bucket_pop);
                const char * value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                } else {
                    value = bucket[bucket_pop];
                    if (++bucket_pop == num_els) {
                        bucket_pop = 0;
                    }
                }
                if (i < this->e_size) {
                    bucket[bucket_push] = this->buf[i]; 
                    if (++bucket_push == num_els) {
                        bucket_push = 0;
                    }
                }
                addElement(new_buf, i, value);
            }
            delete[] bucket;

            if (recapped) {
                delete[] this->buf;
                this->buf = new_buf;
            }
            this->e_size += num_els;

            for (size_t i = 0; i < this->e_size; i++) {
                 loggerPrintf(LOGGER_DEBUG, "%s\n", this->buf[i]);
            }

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos >= this->e_size) return OPERATION_ERROR;

            for (size_t i = pos + num_els; i < this->e_size; i++) {
                this->buf[i - num_els] = this->buf[i];
            }
            this->e_size -= num_els;

            return OPERATION_SUCCESS;
        }
        bool contains(const char * el) {
            for (size_t i = 0; i < this->e_size; i++) {
                if (strcmp(this->buf[i], el) == 0) {
                    return true;
                }
            }
            return false;
        }
        std::string toString() {
            char * nul = {0};
            this->append(nul);
            return std::string((char *)this->buf);
        }
        char * operator [] (const size_t pos) {
            return this->buf[pos];
        }
};
}
#endif 