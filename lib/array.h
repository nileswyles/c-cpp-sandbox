#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdexcept>

#include "result.h"

#ifndef LOGGER_ARRAY
#define LOGGER_ARRAY 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ARRAY
#include "logger.h"

namespace WylesLibs {

constexpr size_t ARRAY_RECOMMENDED_INITIAL_CAP = 8;
constexpr double UPSIZE_FACTOR = 1.75;
constexpr double DOWNSIZE_FACTOR = 0.50;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

// TODO: exceptions...
template<typename T>
class Array {
    private:
        size_t * e_cap;
        size_t * e_size;
        size_t * instance_count;
        void addElement(T * buf, const size_t pos, T el) {
            buf[pos] = el;
        }
    public:
        T * buf;
        Array(): Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        //  could alternatively use constexpr to statically initialize the array but this is definitely nice to have.
        Array(std::initializer_list<T> list) {
            e_cap = new size_t(list.size() * UPSIZE_FACTOR);
            e_size = new size_t(list.size()); 
            instance_count = new size_t(1);
            buf = newCArray<T>(list.size());

            size_t i = 0;
            for (auto el: list) {
                buf[i++] = el;
            }
        }
        Array(const size_t initial_cap): e_cap(new size_t(initial_cap)), e_size(new size_t(0)), buf(newCArray<T>(initial_cap)), instance_count(new size_t(1)) {}
        ~Array() {
            // printf("Deconstructor called., %p, instance count: %ld\n", this, *this->instance_count);
            // printf("Buf..., %p\n", this->buf);
            (*this->instance_count)--;
            if (*this->instance_count == 0) {
                delete[] buf;
                delete e_cap;
                delete e_size;
                delete instance_count;
            }
        }
        size_t size() {
            return *this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return *this->e_cap;
        }
        Array<T>& uniqueAppend(T& el) {
            if (this->contains(el)) { 
            } else {
                return this->append(&el, 1);
            }
        }
        Array<T>& append(T& el) {
            return this->append(&el, 1);
        }
        Array<T>& append(const T * els, const size_t num_els) {
            return this->insert(this->size(), els, num_els);
        }
        Array<T>& insert(const size_t pos, const T& el) {
            return this->insert(pos, &el, 1);
        }
        Array<T>& insert(const size_t pos, const T * els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }

            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->size(), this->cap(), pos);

            T * new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->size() > this->cap()) {
                size_t new_cap = (size_t)((num_els + this->size()) * UPSIZE_FACTOR);
                new_buf = newCArray<T>(new_cap);
                if (new_buf == nullptr) {
                    // if no bad_alloc thrown? lol whatever...
                    std::string msg = "Failed to allocate new array.";
                    loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    recapped = true;
                    *this->e_cap = new_cap;
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
                // if no bad_alloc thrown? lol whatever...
                std::string msg = "Failed to allocate new array.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            size_t bucket_push = 0;
            size_t bucket_pop = 0;
            for (size_t i = pos; i < this->size() + num_els; i++) {
                T value;
                if (i < pos + num_els) {
                    value = els[i - pos];
                } else {
                    value = bucket[bucket_pop];
                    if (++bucket_pop == num_els) {
                        bucket_pop = 0;
                    }
                }
                if (i < this->size()) {
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
            *this->e_size += num_els;

            return *this;
        }
        Array<T>& remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        Array<T>& remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos + num_els > this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }

            T * selected_buf = nullptr;
            bool recapped = false;
            size_t new_cap_threshold = this->cap() * DOWNSIZE_FACTOR;
            size_t potential_new_cap = ((this->size() - num_els) * UPSIZE_FACTOR);
            if (potential_new_cap < new_cap_threshold) {
                T * new_buf = newCArray<T>(potential_new_cap);
                // because of how new/delete work and we want to make sure the data is continuous, we'll need to reallocate...
                if (new_buf == nullptr) {
                    // if no bad_alloc thrown? lol whatever...
                    std::string msg = "Failed to allocate new array.";
                    loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    recapped = true;
                    *this->e_cap = potential_new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automatically intialized by remove operation... 
                    selected_buf = new_buf;
                    for (size_t i = 0; i < pos; i++) {
                        selected_buf[i] = this->buf[i];
                    }
                }
            } else {
                // else, just remove, don't recap array...
                selected_buf = this->buf;
            }
            for (size_t i = pos; i < this->size(); i++) {
                if (i + num_els < this->size()) {
                    // if removing last element, just leave it... decrementing size should be enough...
                    selected_buf[i] = this->buf[i + num_els];
                }
            }
            if (recapped) {
                delete[] this->buf;
                this->buf = selected_buf;
            }

            *this->e_size -= num_els;

            return *this;
        }
        bool contains(const T& el) {
            return find(el) != -1;
        }
        size_t find(const T& el) {
            for (size_t i = 0; i < this->size(); i++) {
                if (this->buf[i] == el) {
                    return i;
                }
            }
            return -1;
        }
        T& at(const size_t pos) {
            if (pos > 0 && pos < this->size()) {
                return this->buf[pos];
            } else {
                // throw std::runtime_error("Invalid pos...");
            }
        }
        T& front() {
            if (this->size() == 0) {
                T t;
                this->append(t);
            }
            return this->buf[0];
        }
        T& back() {
            if (this->size() == 0) {
                T t;
                this->append(t);
            }
            return this->buf[this->size()-1];
        }
        Array<T>& removeFront() {
            remove(0);
            return *this;
        }
        Array<T>& removeBack() {
            remove(this->size()-1);
            return *this;
        }
        std::string toString() {
            T nul = {0};
            this->append(nul);
            return std::string((char *)this->buf);
        }
        T& operator[] (const size_t pos) {
            return this->buf[pos];
        }
        T& operator[] (const T& el) {
            size_t i = this->find(el);
            if (i == -1) {
                this->append(el); 
                this->buf[this->size()-1];
            } else {
                return this->buf[i];
            }
        }
        // copy... 
        //  Can access private variables?
        Array(const Array<T>& x): e_cap(x.e_cap), e_size(x.e_size), buf(x.buf) {
            instance_count = x.instance_count;
            (*this->instance_count)++;
        }
        Array<T>& operator= (const Array<T>& x) {
            this->instance_count = x.instance_count;
            this->buf = x.buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;

            (*this->instance_count)++;
            return *this;
        }
        // Move
        // Array(T&& x) : e_cap(x.cap()), e_size(x.size()), buf(x.buf) {}
        // T& operator= (T&& x) {
        //     this->buf = x.buf; // lol.. this seems like not the right way to do this?
        //     //  let's find out...
        //     this->cap() = x.cap();
        //     this->size() = x.size();
        //     return *this;
        // }
};

template<>
class Array<const char *> {
    private:
        size_t * e_cap;
        size_t * e_size;
        size_t * instance_count;
        void addElement(char ** buffer, const size_t pos, const char * el) {
            char * new_cstring = newCArray<char>(strlen(el) + 1);
            strcpy(new_cstring, el);
            loggerPrintf(LOGGER_DEBUG, "New String: %s\n", new_cstring);
            buffer[pos] = new_cstring;
        }
    public:
        char ** buf;
        Array() : Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        Array(const size_t initial_cap) : e_cap(new size_t(initial_cap)), e_size(new size_t(0)), buf(newCArray<char *>(initial_cap)), instance_count(new size_t(1)) {}
        ~Array() {
            // printf("Deconstructor called., %p, instance count: %ld\n", this, *this->instance_count);
            // printf("Buf..., %p\n", this->buf);
            (*this->instance_count)--;
            if (*this->instance_count == 0) {
                for (size_t i = 0; i < this->size(); i++) {
                    delete[] buf[i];
                }
                delete[] buf;
                delete e_cap;
                delete e_size;
                delete instance_count;
            }
        }
        size_t size() {
            return *this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return *this->e_cap;
        }
        Array<const char *>& append(const char * el) {
            return this->append(&el, 1);
        }
        Array<const char *>& append(const char ** els, const size_t num_els) {
            return this->insert(this->size(), els, num_els);
        }
        Array<const char *>& insert(const size_t pos, const char * el) {
            return this->insert(pos, &el, 1);
        }
        Array<const char *>& insert(const size_t pos, const char ** els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos >= this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }

            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->size(), this->cap(), pos);

            char ** new_buf = this->buf; 
            bool recapped = false;
            if (num_els + this->size() > this->cap()) {
                size_t new_cap = (size_t)((num_els + this->size()) * UPSIZE_FACTOR);
                new_buf = newCArray<char *>(new_cap);
                if (new_buf == nullptr) {
                    // if no bad_alloc thrown? lol whatever...
                    std::string msg = "Failed to allocate new array.";
                    loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    recapped = true;
                    *this->e_cap = new_cap;
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
                // if no bad_alloc thrown? lol whatever...
                std::string msg = "Failed to allocate new array.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            size_t bucket_push = 0;
            size_t bucket_pop = 0;
            for (size_t i = pos; i < this->size() + num_els; i++) {
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
                if (i < this->size()) {
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
            *this->e_size += num_els;

            for (size_t i = 0; i < this->size(); i++) {
                 loggerPrintf(LOGGER_DEBUG, "%s\n", this->buf[i]);
            }

            return *this;
        }
        Array<const char *>& remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        Array<const char *>& remove(const size_t pos, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos + num_els > this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }

            char ** selected_buf = nullptr;
            bool recapped = false;
            size_t new_cap_threshold = this->cap() * DOWNSIZE_FACTOR;
            size_t potential_new_cap = ((this->size() - num_els) * UPSIZE_FACTOR);
            if (potential_new_cap < new_cap_threshold) {
                char ** new_buf = newCArray<char *>(potential_new_cap);
                // because of how new/delete work and we want to make sure the data is continuous, we'll need to reallocate...
                if (new_buf == nullptr) {
                    // if no bad_alloc thrown? lol whatever...
                    std::string msg = "Failed to allocate new array.";
                    loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    recapped = true;
                    *this->e_cap = potential_new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automatically intialized by remove operation... 
                    selected_buf = new_buf;
                    for (size_t i = 0; i < pos; i++) {
                        selected_buf[i] = this->buf[i];
                    }
                }
            } else {
                // else, just remove, don't recap array...
                selected_buf = this->buf;
            }
            for (size_t i = pos; i < this->size(); i++) {
                if (i + num_els < this->size()) {
                    // if removing last element, just leave it... decrementing size should be enough...
                    selected_buf[i] = this->buf[i + num_els];
                }
            }
            if (recapped) {
                delete[] this->buf;
                this->buf = selected_buf;
            }

            *this->e_size -= num_els;

            return *this;
        }
        bool contains(const char * el) {
            for (size_t i = 0; i < this->size(); i++) {
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
        Array(const Array<const char *>& x) : e_cap(x.e_cap), e_size(x.e_size), buf(x.buf) {
            instance_count = x.instance_count;
            (*this->instance_count)++;
        }
        Array<const char *>& operator= (const Array<const char *>& x) {
            this->instance_count = x.instance_count;
            this->buf = x.buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;

            (*this->instance_count)++;
            return *this;
        }
};
}
#endif 