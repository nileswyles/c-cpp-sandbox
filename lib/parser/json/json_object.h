#ifndef WYLESLIBS_JSON_OBJECT_H
#define WYLESLIBS_JSON_OBJECT_H

#include "parser/json/json_mix.h"
#include "parser/json/json_parser.h"
#include "parser/json/json_array.h"
#include "parser/json/json_node.h"
#include "parser/json/jstring.h"

#include <iterator>
#include <algorithm>
#include <vector>
#include <string>

#include "stdint.h"

namespace WylesLibs::Parser::Json {
    // # hybrid-hack!
    //  assuming quite a few things but would be nice. Does uniform initialization support overloading?

    // "compiler-json"
    /*
        {
            {key, true},
            {key, false},
            {key, "str"},
            {key, {
                {key, "str"},
                {key, {
                    {key, "lol"}
                }}
            }},
            {key, [bool, bool, bool]},
            {key, 1},
            {key, 127.0}
        }
    */
    class JsonObject: public JsonValue {
        private:
            std::vector<std::string> keys; 

            size_t find(std::string key) {
                size_t e = SIZE_MAX;
                for (size_t i = 0; i < this->keys.size(); i++) {
                    if (key == this->keys[e]) {
                        e = i;
                    }
                }
                return e;
            }
                                                // iterator_category, value_type, difference_type, pointer, reference
            class iterator: public std::iterator<std::bidirectional_iterator_tag, size_t, size_t, size_t*, size_t> {
                private:
                    size_t i;
                    bool reverse;

                    // TODO: I think it technically points to the same underlying object regardless?
                    std::vector<std::string>& keys;
                    std::vector<JsonValue *>& values;
                public:
                    explicit iterator(std::vector<std::string>& k, std::vector<JsonValue *>& v, size_t _i = 0, bool reverse = false) : keys(k), values(v), i(_i), reverse(reverse) {
                        // the "default" end of the typical collection type points to the element past the last element in the collection.
                        //  (since c++17?) this needs to iterate at least one past the last element in the collection.
                        if (i > values.size()) { // -1 == SIZE_MAX, -2 == SIZE_MAX - 1, 3 == SIZE_MAX - 2
                            throw std::runtime_error("Fail fast.");
                        }
                    }
         
                    // pre increment
                    iterator& operator++() { 
                        if (0 <= i && i <= values.size() - 1) {
                            if (true == reverse) {
                                i--;
                            } else {
                                i++;
                            }
                        }
                        return *this; 
                    }
                    // post increment
                    iterator operator++(int) {
                        iterator it = *this; 
                        if (true == reverse) {
                            --(*this);
                        } else {
                            ++(*this);
                        }
                        return it;
                    }
                    iterator& operator--() { 
                        if (0 <= i && i <= values.size() - 1) {
                            if (true == reverse) {
                                i++;
                            } else {
                                i--;
                            }
                        }
                        return *this; 
                    }
                    iterator operator--(int) {
                        iterator it = *this; 
                        if (true == reverse) {
                            ++(*this);
                        } else {
                            --(*this);
                        }
                        return it;
                    }
                    bool operator==(iterator other) const {
                        return this->i == other.i; 
                    }
                    bool operator!=(iterator other) const {
                        return !(*this == other); 
                    }
                    std::pair<std::string, JsonValue *> operator*() const {
                        return std::pair<std::string, JsonValue *>(keys.at(this->i), values.at(this->i));
                    }
            };
        public:
            size_t depth;
            //  This is really only designed this way to simplify the json parsing and invocation code.
            JsonArray values;
            JsonObject(): JsonObject(0) {}
            JsonObject(std::initializer_list<JsonNode> compile): JsonObject(compile, 0) {}
            JsonObject(std::initializer_list<JsonNode> compile, size_t depth);
            JsonObject(size_t depth);

            ~JsonObject() override = default;

            std::string toJsonString() final override;

            iterator begin(size_t s = 0) {
                return iterator(this->keys, this->values, s);
            }
            iterator end() {
                return iterator(this->keys, this->values, this->values.size());
            }
            iterator end(size_t e) {
                return iterator(this->keys, this->values, e);
            }

            iterator rbegin() {
                return iterator(this->keys, this->values, this->values.size() - 1, true);
            }
            iterator rbegin(size_t s) {
                return iterator(this->keys, this->values, s, true);
            }
            iterator rend(size_t e = SIZE_MAX) {
                return iterator(this->keys, this->values, e, true);
            }

            void addNode(std::string key, bool value);
            void addNode(std::string key, double value);
            void addNode(std::string key, jstring value);
            void addNode(std::string key, JsonArray * value);
            void addNode(std::string key, JsonObject * value);

            void removeNode(std::string key);

            // This along with operator overloading of other JsonValue types provides a way of working with Json. For simple tasks I suppose?
            JsonValue * at(std::string key);
            JsonValue *& operator[] (std::string key) {
                size_t i = this->find(key);
                return i == SIZE_MAX ? this->values[this->values.size()] : this->values[i];
            }

            // ! IMPORTANT - Use at your own risk. 
            //   Accessing using operations defined above after adding a key but before adding a value will result in defined but potentially unexpected behavior
            //   This is really only designed this way to simplify the json parsing and invocation code.

            //   Values should be set using the [] operator above.
            void addKey(std::string key);
    };
}
#endif