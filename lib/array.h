#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include "result.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "logger.h"

#define ARRAY_RECOMMENDED_INITIAL_CAP 8

namespace WylesLibs {
static constexpr double RESIZE_FACTOR = 1.75;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

template<typename T>
class Array {
    private:
        size_t cap;
        void addElement(T * buf, const size_t pos, T el) {
            buf[pos] = el;
        }
    public:
        T * buf;
        size_t size;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<T>(initial_cap)) {}
        ~Array() {
            delete[] buf;
        }
        operation_result append(T& el) {
            return this->append(&el, 1);
        }
        operation_result append(const T * els, const size_t num_els) {
            return this->insert(this->size, els, num_els);
        }
        operation_result insert(const size_t pos, const T& el) {
            return this->insert(pos, el, 1);
        }
        operation_result insert(const size_t pos, const T * els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            logger_printf(LOGGER_DEBUG, "num_els: %ld, size: %ld, cap: %ld, pos: %ld\n", num_els, this->size, this->cap, pos);

            T * new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->size > this->cap) {
                size_t new_cap = (size_t)((num_els + this->size) * RESIZE_FACTOR);
                new_buf = newCArray<T>(new_cap);
                if (new_buf == nullptr) {
                    return MEMORY_OPERATION_ERROR;
                } else {
                    recapped = true;
                    this->cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically intialized by insert operation... (see use new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(T);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = this->buf[i];
                    }
                }
            }
            
            T * temp_buff = newCArray<T>(num_els);
            if (temp_buff == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t temp_push = 0;
            size_t temp_pop = 0;
            for (size_t i = pos; i < this->size + num_els; i++) {
                T value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                } else {
                    value = temp_buff[temp_pop];
                    if (++temp_pop == num_els) {
                        temp_pop = 0;
                    }
                }
                if (i < this->size) {
                    temp_buff[temp_push] = this->buf[i];
                    if (++temp_push == num_els) {
                        temp_push = 0;
                    }
                }
                addElement(new_buf, i, value);
            }
            delete[] temp_buff;

            if (recapped) {
                delete[] this->buf;
                this->buf = new_buf;
            }
            this->size += num_els;

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos >= this->size) return OPERATION_ERROR;

            for (size_t i = pos + num_els; i < this->size; i++) {
                this->buf[i - num_els] = this->buf[i];
            }
            this->size -= num_els;

            return OPERATION_SUCCESS;
        }
        std::string toString() {
            T nul = {0};
            this->append(nul);
            return std::string((char *)this->buf);
        }
        T& operator [] (const size_t pos) {
            return this->buf[pos];
        }
};

template<>
class Array<const char *> {
    private:
        size_t cap;
        void addElement(char ** buffer, const size_t pos, const char * el) {
            char * new_cstring = newCArray<char>(strlen(el) + 1);
            strcpy(new_cstring, el);
            logger_printf(LOGGER_DEBUG, "New String: %s\n", new_cstring);
            buffer[pos] = new_cstring;
        }
    public:
        char ** buf;
        size_t size;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : cap(initial_cap), size(0), buf(newCArray<char *>(initial_cap)) {}
        ~Array() {
            for (size_t i = 0; i < size; i++) {
                delete[] buf[i];
            }
            delete[] buf;
        }
        operation_result append(const char * el) {
            return this->append(&el, 1);
        }
        operation_result append(const char ** els, const size_t num_els) {
            return this->insert(this->size, els, num_els);
        }
        operation_result insert(const size_t pos, const char * el) {
            return this->insert(pos, &el, 1);
        }
        operation_result insert(const size_t pos, const char ** els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size) return OPERATION_ERROR;

            logger_printf(LOGGER_DEBUG, "num_els: %ld, size: %ld, cap: %ld, pos: %ld\n", num_els, this->size, this->cap, pos);

            char ** new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->size > this->cap) {
                size_t new_cap = (size_t)((num_els + this->size) * RESIZE_FACTOR);
                new_buf = newCArray<char *>(new_cap);
                if (new_buf == nullptr) {
                    return MEMORY_OPERATION_ERROR;
                } else {
                    recapped = true;
                    this->cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically intialized by insert operation... (see use new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(char *);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = this->buf[i];
                    }
                }
            }

            char ** temp_buff = newCArray<char *>(num_els);
            if (temp_buff == nullptr) {
                return MEMORY_OPERATION_ERROR;
            }
            size_t temp_push = 0;
            size_t temp_pop = 0;
            for (size_t i = pos; i < this->size + num_els; i++) {
                logger_printf(LOGGER_DEBUG, "push: %ld pop: %ld\n", temp_push, temp_pop);
                const char * value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                } else {
                    value = temp_buff[temp_pop];
                    if (++temp_pop == num_els) {
                        temp_pop = 0;
                    }
                }
                if (i < this->size) {
                    temp_buff[temp_push] = this->buf[i]; 
                    if (++temp_push == num_els) {
                        temp_push = 0;
                    }
                }
                addElement(new_buf, i, value);
            }
            delete[] temp_buff;

            if (recapped) {
                delete[] this->buf;
                this->buf = new_buf;
            }
            this->size += num_els;

            for (size_t i = 0; i < this->size; i++) {
                 logger_printf(LOGGER_DEBUG, "%s\n", this->buf[i]);
            }

            return OPERATION_SUCCESS;
        }
        operation_result remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        operation_result remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos >= this->size) return OPERATION_ERROR;

            for (size_t i = pos + num_els; i < this->size; i++) {
                this->buf[i - num_els] = this->buf[i];
            }
            this->size -= num_els;

            return OPERATION_SUCCESS;
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