#ifndef ARRAY_H
#define ARRAY_H
#include <string>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "array.h"
#include "result.h"
#include "logger.h"

namespace WylesLibs {

// TODO:
//  then hash map, eventually... just for kicks... it's a more complicated implementation.
template<typename K, typename V>
class ArrayMap {
    private:
        size_t size;
        Array<K> keys;
        Array<V> values;
    public:
        ArrayMap() : ArrayMap(ARRAY_RECOMMENDED_INITIAL_CAP) {}
        ArrayMap(const size_t initial_cap) : size(0)) {}
        size_t getSize() {
            return this->size;
        }
        operation_result add(K& key, V& value) {
            V value;
            return add(key, value);
        }
        operation_result add(K& key, V& value) {
            operation_result result = this->keys.uniqueAppend(key)
            if (OPERATION_SUCCESS != result) {
                return result;
            }
            this->values.append(value);
            if (OPERATION_SUCCESS != result) {
                this->keys.remove(this-keys.getSize()-1);
            }
            return result;
        }
        operation_result remove(K& key) {
            const size_t index = this->keys.find(el);
            operation_result result = this->keys.remove(index);
            if (OPERATION_SUCCESS != result) {
                return result;
            }
            result = this->values.remove(index);
            return result;
        }
        bool containsKey(const K& el) {
            return this->keys.contains(el);
        }
        bool containsValue(const V& el) {
            return this->values.contains(el);
        }
        V& operator [] (const K& el) {
            // get key index, then get corresponding value...
            const size_t index = this->keys.find(el);
            if (index == -1) return nullptr; // ?
            return this->values[index];
        }
};
}
#endif 