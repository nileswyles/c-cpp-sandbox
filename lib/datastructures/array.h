#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdexcept>

// make sure global logger level is initialized
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_ARRAY
#define LOGGER_LEVEL_ARRAY GLOBAL_LOGGER_LEVEL
#endif

#ifndef LOGGER_ARRAY
#define LOGGER_ARRAY 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ARRAY

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_ARRAY
#include "logger.h"

namespace WylesLibs {

constexpr size_t ARRAY_RECOMMENDED_INITIAL_CAP = 8;
constexpr double UPSIZE_FACTOR = 1.75;
constexpr double DOWNSIZE_FACTOR = 0.50;

template<typename T>
static inline T * newCArray(size_t size) {
    return (T *) new T[size];
}

template<typename T>
void addElement(T * buf, const size_t pos, T el) {
    buf[pos] = el;
    // loggerPrintf(LOGGER_DEBUG, "%p, NEW BUFFER ELEMENT: [%x], OLD BUFFER ELEMENT: [%x]\n", e_buf + pos, e_buf[pos], el);
}
template<>
void addElement<const char *>(const char ** buf, const size_t pos, const char * el);

template<typename T>
void deleteCArray(T ** e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'generic' of size: %lu\n", size);
    if (e_buf != nullptr) {
        if (*e_buf != nullptr) {
            // deletes array of pointers to object of type T
            delete[] *e_buf;
        }
        // deletes container (pointer to array deleted above) 
        delete e_buf;
    }
}
template<>
void deleteCArray<const char *>(const char *** e_buf, size_t size);

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
template<typename T>
ssize_t arrayFind(T ** e_buf, size_t size, const T el) {
    for (size_t i = 0; i < size; i++) {
        if ((*e_buf)[i] == el) {
            return i;
        }
    }
    return -1;
}
template<>
ssize_t arrayFind<const char *>(const char *** e_buf, size_t size, const char * el);

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

// TODO: thread safety
//  also, this is probably the more correct way of doing this but could have alternatively used unique_ptr instead of containerizing?
template<typename T>
class Array {
    protected:
        size_t * instance_count;

        T ** e_buf;
        size_t * e_cap;
        size_t * e_size;
        ArraySort * e_sorted;
        // TODO: kind of annoying (yet another unknown?) but let's assume these are defaulted to false == 0.
        bool destructed;
        bool constructed;

        void nlognsortMerge(T * A, size_t size_a, T * B, size_t size_b, T * swap_space) {
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
                if (j < size_b && nlognsortCompare<T>(*this->e_sorted, left_compare, B[j]) > 0) {
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
                if (j < size_b && nlognsortCompare<T>(*this->e_sorted, left_compare, B[j]) > 0) {
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
                // reduce memory usage size, recursion is generally frownd upon!
                size_t size_left = ceil(size/2.0);
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
                            merge(left_buf, span, right_buf, right_size, swap_space);
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
        Array(): Array(ARRAY_RECOMMENDED_INITIAL_CAP) {
            // Only care about implicit compiler fooooooo when copying... so only in default constructor...
            constructed = true;
            destructed = false;
        }
        //  could alternatively use constexpr to statically initialize the array but this is definitely nice to have.
        Array(std::initializer_list<T> list) {
            instance_count = new size_t(1);
            e_cap = new size_t(list.size() * UPSIZE_FACTOR);
            e_buf = new T *(newCArray<T>(*e_cap));
            e_size = new size_t(list.size()); 
            e_sorted = new ArraySort(ARRAY_SORT_UNSORTED);

            size_t i = 0;
            for (auto el: list) {
                (*e_buf)[i++] = el;
            }
            constructed = true;
            destructed = false;
        }
        Array(const size_t initial_cap) {
            instance_count = new size_t(1);
            e_cap = new size_t(initial_cap);
            e_buf = new T*(newCArray<T>(*e_cap));
            e_size = new size_t(0);
            e_sorted = new ArraySort(ARRAY_SORT_UNSORTED);
            constructed = true;
            destructed = false;
        }
         Array(size_t * instance_count, size_t * e_buf, size_t * e_cap, size_t * e_size, size_t * e_sorted) {
            instance_count = instance_count;
            e_buf = e_buf;
            e_cap = e_cap;
            e_size = e_size;
            e_sorted = e_sorted;
            constructed = true;
            destructed = false;
        }
        virtual ~Array() {
            if (false == this->destructed) {
                (*this->instance_count)--;
                if (*this->instance_count == 0) {
                    deleteCArray<T>(e_buf, *e_size);
                    if (instance_count != nullptr) {
                        delete instance_count;
                    }
                    if (e_cap != nullptr) {
                        delete e_cap;
                    } 
                    if (e_size != nullptr) {
                        delete e_size;
                    }
                    if (e_sorted != nullptr) {
                        delete e_sorted;
                    }
                }
                this->destructed = true;
            }
        }

        Array<T>& sort(ArraySort sortOrder) {
            if (sortOrder != ARRAY_SORT_UNSORTED && *e_sorted != sortOrder) {
                *e_sorted = sortOrder;
                try {
                    nlognSort(*this->e_buf, *this->e_size);
                } catch (const std::exception& e) {
                    loggerPrintf(LOGGER_ERROR, "%s\n", e.what());
                    // inserts, removes and bad allocs makes it so that we can't assume array is sorted.
                    *e_sorted = ARRAY_SORT_UNSORTED;
                    throw e;
                }
            }
            return *this;
        }
        Array<T>& insert(const size_t pos, const T * els, const size_t num_els) {
            // pos out of bounds, return error...
            if (pos < 0 || pos > this->size()) {
                std::string msg = "Position out of range.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
        
            loggerPrintf(LOGGER_DEBUG, "num_els: %ld, size: %ld, e_cap: %ld, pos: %ld\n", num_els, this->size(), this->cap(), pos);
        
            T * new_buf = *this->e_buf; 
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
                    //  the rest will be automagically initialized by the insert operation... (see use of new_buf vs this->buf variables below)
                    size_t total_size_up_to_pos = pos * sizeof(T);
                    for (size_t i = 0; i < pos; i++) {
                        new_buf[i] = (*this->e_buf)[i];
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
                    bucket[bucket_push] = (*this->e_buf)[i];
                    if (++bucket_push == num_els) {
                        bucket_push = 0;
                    }
                }
                addElement<T>(new_buf, i, value);
            }
            delete[] bucket;
        
            *this->e_size += num_els;
        
            if (recapped) {
                delete[] *this->e_buf;
                *this->e_buf = new_buf;
            }
        
            *e_sorted = ARRAY_SORT_UNSORTED;
        
            return *this;
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
                        selected_buf[i] = (*this->e_buf)[i];
                    }
                }
            } else {
                // else, just remove, don't recap array...
                selected_buf = *this->e_buf;
            }
            for (size_t i = pos; i < this->size(); i++) {
                if (i < pos + num_els) {
                    // make sure to deallocate memory for elements being removed.
                    deleteCArrayElement<T>(*this->e_buf, i);
                }
                // if removing last element, no shifting needed... decrementing size should be enough...
                if (i + num_els < this->size()) {
                    selected_buf[i] = (*this->e_buf)[i + num_els];
                }
            }
            if (recapped) {
                delete[] *this->e_buf;
                *this->e_buf = selected_buf;
            }
        
            *this->e_size -= num_els;
        
            return *this;
        }
        T * buf() {
            // use at your own risk, obviously...
            return *this->e_buf;
        }
        virtual size_t size() {
            return *this->e_size;
        }
        size_t cap() {
            // this is really only useful for testing.
            return *this->e_cap;
        }
        Array<T>& uniqueAppend(const T& el) {
            if (this->contains(el)) { 
            } else {
                return this->append(&el, 1);
            }
        }
        Array<T>& append(const T& el) {
            return this->append(&el, 1);
        }
        Array<T>& append(const T * els, const size_t num_els) {
            return this->insert(this->size(), els, num_els);
        }
        Array<T>& insert(const size_t pos, const T& el) {
            return this->insert(pos, &el, 1);
        }
        Array<T>& removeEl(const T& el) {
            size_t i = this->find(el);
            if (i != -1) {
                remove(i, 1);
            }
            return *this;
        }
        Array<T>& remove(const size_t pos) {
            return this->remove(pos, 1);
        }
        bool contains(const T& el) {
            return this->find(el) != -1;
        }
        ssize_t find(const T& el) {
            return arrayFind<T>(this->e_buf, this->size(), el);
        }
        T& at(const size_t pos) {
            if (pos >= 0 && pos < this->size()) {
                return (*this->e_buf)[pos];
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
            return (*this->e_buf)[0];
        }
        T& back() {
            if (this->size() == 0) {
                std::string msg = "No element to return.";
                loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return (*this->e_buf)[this->size()-1];
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
        std::string toString() {
            T nul = {0};
            this->append(nul);
            return std::string((char *)(*this->e_buf));
        }
        T& operator[] (const size_t pos) {
            // TODO: check if exists... else, append...
            // like other overload... at, checks if exist i belive already.
            size_t i = pos;
            if (pos >= this->size()) {
                T el;
                this->append(el); 
                i = this->size()-1;
            }
            return (*this->e_buf)[i];
        }
        // TODO: so, passing a literal to this is defined by the language? 
        //      even though the literal will never be used again. lol
        //      I've seen this everywhere but if undefined by spec? then maybe not a good idea...
        //      
        //      it at least works for this compiler lol...

        // lol, also const reference? I'm pretty sure I looked into but think about this again... too lazy right now..
        T& operator[] (const T& el) {
            size_t i = this->find(el);
            if (i == -1) {
                this->append(el); 
                i = this->size() - 1;
            }
            return (*this->e_buf)[i];
        }
        T& operator+= (const T& el) {
            this->append(el); 
        }
        // Copy
        Array(const Array<T>& x) {
            printf("ARRAY COPY CONSTRUCTOR COPYING MATRIX VECTOR!!!! LOL\n");
            this->instance_count = x.instance_count;
            this->e_buf = x.e_buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;
            this->e_sorted = x.e_sorted;
         
            this->destructed = false;
            if (false == this->constructed) {
                (*this->instance_count)++;
                this->constructed = true;
            }
        }
        Array<T>& operator= (const Array<T>& x) {
            printf("ARRAY COPY ASSIGNMENT COPYING MATRIX VECTOR!!!! LOL\n");
            this->instance_count = x.instance_count;
            this->e_buf = x.e_buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;
            this->e_sorted = x.e_sorted;
         
            this->destructed = false;
            if (false == this->constructed) {
                (*this->instance_count)++;
                this->constructed = true;
            }
            return *this;
        }
};
}
#endif 