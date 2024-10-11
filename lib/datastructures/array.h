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

// TODO: again, member function specialization didn't work, so...
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

        inline void nlognsortMerge(T * A, size_t size_a, T * B, size_t size_b, T * swap_space);
        void nlognSort(T * e_buf, size_t size);
    public:
        Array(): Array(ARRAY_RECOMMENDED_INITIAL_CAP) {
            // Only care about implicit compiler fooooooo when copying... so only in default constructor...
            this->constructed = true;
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
        }
        Array(const size_t initial_cap) {
            instance_count = new size_t(1);
            e_cap = new size_t(initial_cap);
            e_buf = new T*(newCArray<T>(*e_cap));
            e_size = new size_t(0);
            e_sorted = new ArraySort(ARRAY_SORT_UNSORTED);
        }
         Array(size_t * instance_count, size_t * e_buf, size_t * e_cap, size_t * e_size, size_t * e_sorted) {
            instance_count = instance_count;
            e_buf = e_buf;
            e_cap = e_cap;
            e_size = e_size;
            e_sorted = e_sorted;
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
        Array<T>& sort(ArraySort sortOrder);
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
        Array<T>& insert(const size_t pos, const T * els, const size_t num_els);
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
        Array<T>& remove(const size_t pos, const size_t num_els);
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
            if (false == this->constructed) {
                this->instance_count = x.instance_count;
                this->e_buf = x.e_buf;
                this->e_cap = x.e_cap;
                this->e_size = x.e_size;
                this->e_sorted = x.e_sorted;
         
                (*this->instance_count)++;
                this->constructed = true;
            }
        }
        Array<T>& operator= (const Array<T>& x) {
            if (false == this->constructed) {
                this->instance_count = x.instance_count;
                this->e_buf = x.e_buf;
                this->e_cap = x.e_cap;
                this->e_size = x.e_size;
                this->e_sorted = x.e_sorted;
         
                (*this->instance_count)++;
                this->constructed = true;
            }
            return *this;
        }
};
}
#endif 