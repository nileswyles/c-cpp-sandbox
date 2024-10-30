#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <initializer_list>
#include <stdexcept>
#include <istream>

// make sure global logger level is initialized
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_ARRAY
#define LOGGER_LEVEL_ARRAY GLOBAL_LOGGER_LEVEL
#endif

#ifndef LOGGER_ARRAY
#define LOGGER_ARRAY 0
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ARRAY

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_ARRAY
#include "logger.h"

namespace WylesLibs {

// @

constexpr size_t ARRAY_RECOMMENDED_INITIAL_CAP = 8;
constexpr double UPSIZE_FACTOR = 1.75;
constexpr double DOWNSIZE_FACTOR = 0.50;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

template<typename T>
void addElement(T * buf, const size_t pos, T el) {
    // copy assignment
    buf[pos] = el;
}
template<>
void addElement<const char *>(const char ** buf, const size_t pos, const char * el);

template<typename T>
void deleteCArray(T * e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'generic' of size: %lu\n", size);
    if (e_buf != nullptr) {
        // deletes array of pointers to object of type T
        delete[] e_buf;
    }
}
template<>
void deleteCArray<const char *>(const char ** e_buf, size_t size);

template<typename T>
void deleteCArrayElement(T * buf, size_t pos) {
    // ! IMPORTANT - these types are presumably non-pointer types... if not, then the developer needs to create specialization and write tests (see cstrings).
    //  Not as easy to detect the need for this with functional testing, 
    //      so developer should add a specialization of this if a specialization of addElement is implemented...

    //  TODO: Compiler or lint check for this?
}
template<>
void deleteCArrayElement<const char *>(const char ** buf, size_t pos);

// TODO: again, member function "specialization" didn't work, so... I think the non-specialization stuff was working lol...
//  revisit in future.

// TODO: this should be const.
template<typename T>
ssize_t arrayFind(T * e_buf, size_t size, T el) {
    for (size_t i = 0; i < size; i++) {
        if (e_buf[i] == el) {
            return i;
        }
    }
    return -1;
}
template<>
ssize_t arrayFind<const char *>(const char ** e_buf, size_t size, const char * el);

typedef enum ArraySort {
    ARRAY_SORT_UNSORTED,
    ARRAY_SORT_ASCENDING,
    ARRAY_SORT_DESCENDING
} ArraySort;

template<typename T>
int nlognsortCompare(ArraySort sortOrder, T A, T B) {
    if (A == B) {
        return 0;
    } else if (A > B) {
        if (sortOrder == ARRAY_SORT_DESCENDING) {
            return -1;
        } else {
            return 1;
        }
    } else {
        if (sortOrder == ARRAY_SORT_DESCENDING) {
            return 1;
        } else {
            return -1;
        }
    }
}
template<>
int nlognsortCompare<const char *>(ArraySort sortOrder, const char * A, const char * B);

// @

// TODO: thread safety
template<typename T>
class Array {
    protected:
        T * e_buf;
        size_t e_cap;
        size_t e_size;
        ArraySort e_sorted;

        inline void nlognsortMerge(T * A, size_t size_a, T * B, size_t size_b, T * swap_space) {
            size_t swap_space_push = 0;
            size_t swap_space_pop = 0;

            size_t i = 0;
            size_t j = 0;
            T swap;
            T left_compare;
            while (i < size_a) {
                left_compare = A[i];
                if (swap_space_push - swap_space_pop > 0) {
                    left_compare = swap_space[swap_space_pop];
                }
                if (j < size_b && nlognsortCompare<T>(this->e_sorted, left_compare, B[j]) > 0) {
                    // B wins
                    swap = A[i];
                    A[i] = B[j];
                    swap_space[swap_space_push++] = swap;
                    j++;
                } else if (swap_space_push - swap_space_pop > 0) {
                    // swap space wins
                    swap = A[i];
                    A[i] = swap_space[swap_space_pop++];
                    // set new value at end of swap space
                    swap_space[swap_space_push++] = swap;
                } // else swap_space empty and A wins
                i++;
            }
            // merge swap space with remaining B, remember assuming contigious
            while (swap_space_push - swap_space_pop > 0) {
                left_compare = swap_space[swap_space_pop];
                if (j < size_b && nlognsortCompare<T>(this->e_sorted, left_compare, B[j]) > 0) {
                    // by law of numbers i will never be more than j lol
                    A[i] = B[j];
                    j++;
                } else {
                    // swap space wins
                    A[i] = swap_space[swap_space_pop++];
                    // note, size of swap space remains the same...
                } // else swap_space empty and A wins
                i++;
            }
            return;
        }
        void nlognSort(T * e_buf, size_t size) {
            if (false == (e_buf == nullptr || size <= 1)) {
                // reduce memory usage - recursion is generally frownd upon!
                // ! IMPORTANT - kind of not obvious, but in an effort to optimize both runtime and memory
                //   we need a bucket of at least half the size of the array to swap elements in merge function.
                //   floor(half) is (at least) more than the maximum size of the right buffer, because right_size is set to span which is a power of 2...
                size_t size_left = floor(size/2.0);
                T * swap_space = new T[size_left];
                size_t span = 1;
                T * left_buf;
                T * right_buf;
                // so, this basically skips the first half of the tree... let's not bother updating other sort stuff because visualization..
                while (span < size) {
                    size_t i = 0;
                    while (i < size) {
                        left_buf = e_buf + i;
                        if (i + span < size) {
                            // if right buf is within bounds... 
                            //  else it's the odd element out (last element, so adhocly bring in the last odd element in later iterations.) 
                            right_buf = e_buf + i + span;
                            size_t right_size = span;
                            if (i + span + right_size > size) {
                                right_size = size - (i + span);
                            }
                            // left must always be larger or equal to right
                            nlognsortMerge(left_buf, span, right_buf, right_size, swap_space);
                        }
                        i += (2*span);
                    }
                    span *= 2;
                }
                delete[] swap_space;
            }
            return;
        }
    public:
        Array(): Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        //  could alternatively use constexpr to statically initialize the array but this is definitely nice to have.
        Array(std::initializer_list<T> list) {
            e_cap = list.size() * UPSIZE_FACTOR;
            e_buf = newCArray<T>(e_cap);
            e_size = list.size(); 
            e_sorted = ArraySort(ARRAY_SORT_UNSORTED);

            size_t i = 0;
            for (auto el: list) {
                e_buf[i++] = el;
            }
        }
        Array(const size_t initial_cap) {
            e_cap = initial_cap;
            e_buf = newCArray<T>(e_cap);
            e_size = 0;
            e_sorted = ArraySort(ARRAY_SORT_UNSORTED);
        }
        // TODO: only care for uint8_t or char types for now.
        Array(std::istream<T> stream, size_t size) {
            e_cap = size;
            e_buf = newCArray<T>(e_cap);
            e_size = e_cap;
            e_sorted = ArraySort(ARRAY_SORT_UNSORTED);

            stream->read(e_buf, e_size);
        }
        virtual ~Array() {
            deleteCArray<T>(e_buf, e_size);
        }
        Array<T>& sort(ArraySort sortOrder) {
            if (sortOrder != ARRAY_SORT_UNSORTED && e_sorted != sortOrder) {
                e_sorted = sortOrder;
                try {
                    nlognSort(this->e_buf, this->e_size);
                } catch (const std::exception& e) {
                    loggerPrintf(LOGGER_ERROR, "%s\n", e.what());
                    // inserts, removes and bad allocs makes it so that we can't assume array is sorted.
                    e_sorted = ARRAY_SORT_UNSORTED;
                    throw e;
                }
            }
            return *this;
        }
        // TODO: no overloading?
        Array<T>& insert(const size_t pos, const T * els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos > this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
        
            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->size(), this->cap(), pos);
        
            T * new_buf = this->e_buf; 
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
                    this->e_cap = new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automagically initialized by the insert operation... (see use of new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(T);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = (this->e_buf)[i];
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
                    bucket[bucket_push] = (this->e_buf)[i];
                    if (++bucket_push == num_els) {
                        bucket_push = 0;
                    }
                }
                addElement<T>(new_buf, i, value);
            }
            delete[] bucket;
        
            this->e_size += num_els;
        
            if (recapped) {
                delete[] this->e_buf;
                this->e_buf = new_buf;
            }
        
            e_sorted = ARRAY_SORT_UNSORTED;
        
            return *this;
        }
        Array<T>& insert(const size_t pos, const T& el) {
            return this->insert(pos, &el, 1);
        }
        Array<T>& uniqueAppend(const T& el) {
            if (this->contains(el)) { 
            } else {
                this->append(&el, 1);
            }
            return *this;
        }
        Array<T>& append(const T& el) {
            return this->append(&el, 1);
        }
        Array<T>& append(const Array<T>& x) {
            return this->append(x.e_buf, x.e_size);
        }
        Array<T>& append(const T * els, const size_t num_els) {
            return this->insert(this->size(), els, num_els);
        }
        Array<T>& remove(const size_t pos, const size_t num_els) {
            printf("Array remove(pos, els)\n");
            // pos out of bounds, return error...
            if (pos + num_els > this->size()) {
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
                    this->e_cap = potential_new_cap;
                    // if recapped, copy elements up until pos.
                    //  the rest will be automatically intialized by remove operation... 
                    selected_buf = new_buf;
                    printf("hmm....\n");
                    for (size_t i = 0; i < pos; i++) {
                        selected_buf[i] = (this->e_buf)[i];
                    }
                }
            } else {
                // else, just remove, don't recap array...
                selected_buf = this->e_buf;
            }
            printf("removing!\n");
            for (size_t i = pos; i < this->size(); i++) {
                if (i < pos + num_els) {
                    // make sure to deallocate memory for elements being removed.
                    deleteCArrayElement<T>(this->e_buf, i);
                }
                // if removing last element, no shifting needed... decrementing size should be enough...
                if (i + num_els < this->size()) {
                    selected_buf[i] = (this->e_buf)[i + num_els];
                }
            }
            if (recapped) {
                delete[] this->e_buf;
                this->e_buf = selected_buf;
            }
        
            this->e_size -= num_els;
        
            return *this;
        }
        // TODO:
        //      this should be const, does const reference have some other semantic?
        Array<T>& removeEl(const T& el) {
            ssize_t i = this->find(el);
            if (i != -1) {
                remove(i, 1);
            }
            return *this;
        }
        Array<T>& remove(const size_t pos) {
            printf("Array remove(pos)\n");
            return this->remove(pos, 1);
        }
        Array<T>& removeFront() {
            if (this->size() == 0) {
                std::string msg = "No element to remove.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            remove(0);
            return *this;
        }
        Array<T>& removeBack() {
            if (this->size() == 0) {
                std::string msg = "No element to remove.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            remove(this->size()-1);
            return *this;
        }
        T * start() {
            return this->e_buf;
        }
        T * end() {
            return this->e_buf + this->size();
        }
        size_t size() {
            return this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return this->e_cap;
        }
        bool contains(const T& el) {
            return this->find(el) != -1;
        }
        ssize_t find(const T& el) {
            return arrayFind<T>(this->e_buf, this->size(), el);
        }
        T& at(const size_t pos) {
            if (pos < this->size()) {
                return (this->e_buf)[pos];
            } else {
                throw std::runtime_error("Invalid position.");
            }
        }
        T& front() {
            if (this->size() == 0) {
                std::string msg = "No element to return.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return (this->e_buf)[0];
        }
        T& back() {
            if (this->size() == 0) {
                std::string msg = "No element to return.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return (this->e_buf)[this->size()-1];
        }
        std::string toString() {
            T nul = {0};
            this->append(nul);
            return std::string((char *)(this->e_buf));
        }
        T& operator[] (const size_t pos) {
            size_t i = pos;
            if (i >= this->size()) {
                T el;
                this->append(el); 
                i = this->size()-1;
            }
            return this->e_buf[i];
        }
        // TODO: so, passing a literal to this is defined by the language? 
        //      even though the literal will never be used again. lol
        //      I've seen this everywhere but if undefined by spec? then maybe not a good idea...
        //      
        //      it at least works for this compiler lol...

        // lol, also const reference? I'm pretty sure I looked into but think about this again... too lazy right now..
        T& operator[] (const T& el) {
            size_t i = this->find(el);
            if (i == SIZE_MAX) {
                this->append(el); 
                i = this->size() - 1;
            }
            return (this->e_buf)[i];
        }
        // TODO: += doesn't work?
        Array<T>& operator+ (const Array<T>& x) {
            this->append(x);
            return *this;
        }
};

// @

template<typename T>
class ArrayControl {
    public:
        Array<T> * ptr;
        size_t instance_count;
        ArrayControl(): ptr(new Array<T>), instance_count(1) {}
        //  could alternatively use constexpr to statically initialize the array but this is definitely nice to have.
        ArrayControl(std::initializer_list<T> list): ptr(new Array<T>(list)), instance_count(1) {}
        ArrayControl(const size_t initial_cap): ptr(new Array<T>(initial_cap)), instance_count(1) {}
        ArrayControl(std::istream<T> stream, size_t size): ptr(new Array<T>(stream, size)), instance_count(1) {}
        ~ArrayControl() {
            delete this->ptr;
        }
};

// @

template<typename T> 
class SharedArray {
    protected:
        ArrayControl<T> * ctrl;
    public:
        // ! IMPORTANT 
        // implement same interface as Array...
        SharedArray(): ctrl(new ArrayControl<T>()) {}
        SharedArray(std::initializer_list<T> list): ctrl(new ArrayControl<T>(list)) {}
        SharedArray(const size_t initial_cap): ctrl(new ArrayControl<T>(initial_cap)) {}
        SharedArray(std::istream<T> stream, size_t size): ctrl(new ArrayControl<T>(stream, size)) {}

        virtual ~SharedArray() {
            if (this->ctrl != nullptr) {
                (this->ctrl->instance_count)--;
                if (this->ctrl->instance_count == 0) {
                    delete this->ctrl;
                }
            }
        }
        SharedArray<T>& sort(ArraySort sortOrder) {
            this->ctrl->ptr->sort(sortOrder);
            return *this;
        }
        virtual SharedArray<T>& insert(const size_t pos, const T * els, const size_t num_els) {
            this->ctrl->ptr->insert(pos, els, num_els);
            return *this;
        }
        virtual SharedArray<T>& insert(const size_t pos, const T& el) {
            this->ctrl->ptr->insert(pos, el);
            return *this;
        }
        virtual SharedArray<T>& uniqueAppend(const T& el) {
            this->ctrl->ptr->uniqueAppend(el);
            return *this;
        }
        virtual SharedArray<T>& append(const T& el) {
            this->ctrl->ptr->append(el);
            return *this;
        }
        // TODO:
        //      this should be const, does const reference have some other semantic?
        virtual SharedArray<T>& append(SharedArray<T>& other) {
            this->ctrl->ptr->append(other.start(), other.size());
            return *this;
        }
        virtual SharedArray<T>& append(const T * els, const size_t num_els) {
            this->ctrl->ptr->append(els, num_els);
            return *this;
        }
        virtual SharedArray<T>& remove(const size_t pos, const size_t num_els) {
            this->ctrl->ptr->remove(pos, num_els);
            return *this;
        }
        virtual SharedArray<T>& removeEl(const T& el) {
            this->ctrl->ptr->removeEl(el);
            return *this;
        }
        virtual SharedArray<T>& remove(const size_t pos) {
            printf("SharedArray remove\n");
            this->ctrl->ptr->remove(pos);
            return *this;
        }
        virtual SharedArray<T>& removeFront() {
            this->ctrl->ptr->removeFront();
            return *this;
        }
        virtual SharedArray<T>& removeBack() {
            this->ctrl->ptr->removeBack();
            return *this;
        }
        T * start() {
            return this->ctrl->ptr->start();
        }
        T * end() {
            return this->ctrl->ptr->end();
        }
        virtual size_t size() {
            return this->ctrl->ptr->size();
        }
        size_t cap() {
            // this is really only useful for testing.
            return this->ctrl->ptr->cap();
        }
        bool contains(const T& el) {
            return this->ctrl->ptr->contains(el);
        }
        ssize_t find(const T& el) {
            return this->ctrl->ptr->find(el);
        }
        T& at(const size_t pos) {
            return this->ctrl->ptr->at(pos);
        }
        T& front() {
            return this->ctrl->ptr->front();
        }
        T& back() {
            return this->ctrl->ptr->back();
        }
        std::string toString() {
            return this->ctrl->ptr->toString();
        }
        T& operator[] (const size_t pos) {
            return (*this->ctrl->ptr)[pos];
        }
        T& operator[] (const T& el) {
            return (*this->ctrl->ptr)[el];
        }
        // Copy
        SharedArray(const SharedArray<T>& x) {
            this->ctrl = x.ctrl;
            this->ctrl->instance_count++;
        }
        SharedArray<T>& operator= (const SharedArray<T>& x) {
            this->ctrl = x.ctrl;
            this->ctrl->instance_count++;
            return *this;
        }
        SharedArray<T>& operator+ (const SharedArray<T>& x) {
            this->append(x);
            return *this;
        }
};

// @ static

// // 8 + 8 + 8 + 1 = 25 // 8 + 8 + 8 + 1 = 13
// TODO:
// math's not mathing but if that's what compiler says
static_assert(sizeof(Array<uint8_t>) == 40); 
static_assert(sizeof(SharedArray<uint8_t>) == 16); 

}
#endif 