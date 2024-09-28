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
void addElement(T * e_buf, const size_t pos, T el) {
    e_buf[pos] = el;
    // loggerPrintf(LOGGER_DEBUG, "%p, NEW BUFFER ELEMENT: [%x], OLD BUFFER ELEMENT: [%x]\n", e_buf + pos, e_buf[pos], el);
}
template<>
void addElement<const char *>(const char ** buffer, const size_t pos, const char * el);

template<typename T>
void deleteCArray(T ** e_buf, size_t size) {
    loggerPrintf(LOGGER_DEBUG, "Deleting C Array of type 'generic'\n");
    delete[] *e_buf;
    delete e_buf;
}
template<>
void deleteCArray<void *>(void *** e_buf, size_t size);
template<>
void deleteCArray<const char *>(const char *** e_buf, size_t size);

template<typename T>
void deleteCArrayElement(T * e_buf, size_t pos) {}
template<>
void deleteCArrayElement<const char *>(const char ** e_buf, size_t pos);
template<>
void deleteCArrayElement<void *>(void ** e_buf, size_t pos);

// TODO: again, member function specialization didn't work, so...
//  revisit in future.
template<typename T>
size_t arrayFind(T ** e_buf, size_t size, const T el) {
    for (size_t i = 0; i < size; i++) {
        if ((*e_buf)[i] == el) {
            return i;
        }
    }
    return -1;
}
template<>
size_t arrayFind<const char *>(const char *** e_buf, size_t size, const char * el);

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
template<typename T>
class Array {
    private:
        size_t * instance_count;

        T ** e_buf;
        size_t * e_cap;
        size_t * e_size;
        ArraySort * e_sorted;

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
        }

        void nlognSort(T * e_buf, size_t size, T * ss) {
            if (e_buf == nullptr || size <= 1) {
                return;
            } else {
                // ensure left always larger than right
                size_t size_left = ceil(size/2.0);
                size_t size_right = size - size_left;
                T * left_buf = e_buf;
                T * right_buf = e_buf + size_left;
                loggerPrintf(LOGGER_DEBUG, "CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
                T * swap_space = ss;
                if (ss == nullptr) {
                    swap_space = new T[size_left];
                }
                nlognSort(left_buf, size_left, swap_space); // left
                nlognSort(right_buf, size_right, swap_space); // right
                nlognsortMerge<T>(left_buf, size_left, right_buf, size_right, swap_space);
                if (ss == nullptr) {
                    delete[] swap_space;
                }
                loggerPrintf(LOGGER_DEBUG, "CALL TRACE merged size: %ld\n", size);
            }
        }
    public:
        Array(): Array(ARRAY_RECOMMENDED_INITIAL_CAP) {}
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
        }
        Array(const size_t initial_cap) {
            instance_count = new size_t(1);
            e_cap = new size_t(initial_cap);
            e_buf = new T*(newCArray<T>(*e_cap));
            e_size = new size_t(0);
            e_sorted = new ArraySort(ARRAY_SORT_UNSORTED);
        }
        ~Array() {
            (*this->instance_count)--;
            if (*this->instance_count == 0) {
                delete instance_count;
                delete e_cap;
                delete e_size;
                delete e_sorted;
                deleteCArray<T>(e_buf, *e_size);
            }
        }
        Array<T>& sort(ArraySort sortOrder) {
            if (sortOrder != ARRAY_SORT_UNSORTED && *e_sorted != sortOrder) {
                *e_sorted = sortOrder;
                // see if class member specialization was working then no games of telephone...
                //  hmm....

                // AH!

                // can telephone or or or or or or or or

                //  eh idk, there is likely a valid argument for telephoning?
                try {
                    nlognSort(*this->e_buf, *this->e_size, nullptr);
                } catch (const std::exception& e) {
                    loggerPrintf(LOGGER_ERROR, "%s\n", e.what());
                    // TODO:
                    // we might want to handle bad_allocs differently?... consider insert and remove when I think about this again.
                    *e_sorted = ARRAY_SORT_UNSORTED;
                    throw e;
                }
            }
            return *this;
        }
        T * buf() {
            // use at your own risk, obviously...
            return *this->e_buf;
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
                        selected_buf[i] = (*this->e_buf)[i];
                    }
                }
            } else {
                // else, just remove, don't recap array...
                selected_buf = *this->e_buf;
            }
            for (size_t i = pos; i < this->size(); i++) {
                if (i + num_els < this->size()) {
                    // if removing last element, just leave it... decrementing size should be enough...
                    selected_buf[i] = (*this->e_buf)[i + num_els];
                    deleteCArrayElement<T>(*this->e_buf, i);
                }
            }
            if (recapped) {
                delete[] *this->e_buf;
                *this->e_buf = selected_buf;
            }

            *this->e_size -= num_els;

            return *this;
        }
        bool contains(const T& el) {
            return arrayFind<T>(this->e_buf, this->size(), el) != -1;
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
            return (*this->e_buf)[pos];
        }
        T& operator[] (const T& el) {
            size_t i = this->find(el);
            if (i == -1) {
                this->append(el); 
                (*this->e_buf)[this->size()-1];
            } else {
                return (*this->e_buf)[i];
            }
        }
        // Copy
        Array(const Array<T>& x) {
            this->instance_count = x.instance_count;
            this->e_buf = x.e_buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;
            this->e_sorted = x.e_sorted;

            (*this->instance_count)++;
        }
        Array<T>& operator= (const Array<T>& x) {
            this->instance_count = x.instance_count;
            this->e_buf = x.e_buf;
            this->e_cap = x.e_cap;
            this->e_size = x.e_size;
            this->e_sorted = x.e_sorted;

            (*this->instance_count)++;
            return *this;
        }
        // Move
        // Array(Array<T>&& x) {
        //     this->instance_count = x.instance_count;
        //     this->e_buf = x.e_buf;
        //     this->e_cap = x.e_cap;
        //     this->e_size = x.e_size;
        //     this->e_sorted = x.e_sorted;

        //     (*this->instance_count)++;
        // }
        // Array<T>& operator= (Array<T>&& x) {
        //     this->instance_count = x.instance_count;
        //     this->e_buf = x.e_buf;
        //     this->e_cap = x.e_cap;
        //     this->e_size = x.e_size;
        //     this->e_sorted = x.e_sorted;

        //     (*this->instance_count)++;
        //     return *this;
        // }
};
}
#endif 