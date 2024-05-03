#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include "result.h"

#define ARRAY_RECOMMENDED_INITIAL_CAP 8

namespace WylesLibs {
template <class T>
static inline T * newCArray(size_t size) {
    return (T *) ::operator new (sizeof(T) * size);
}

// hm......
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
}
#endif 