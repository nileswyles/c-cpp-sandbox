#ifndef WYLESLIBS_JSON_NODE_H
#define WYLESLIBS_JSON_NODE_H

#include "jstring.h"
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
    class JsonNode {
        public:
            std::string key;
            bool * boolean_value;
            // ! IMPORTANT - the literal should be of type jstring... in other words I think L"" for wchar_t?
            jstring * string_value;
            double * number_value;
            std::vector<JsonNode> * vector_value;
            std::initializer_list<JsonNode> object_value;

            // value
            JsonNode(bool value): boolean_value(new bool(value)) {}
            JsonNode(jstring value): string_value(new jstring(value)) {}
            JsonNode(double value): number_value(new double(value)) {}
            JsonNode(std::vector<JsonNode> value): vector_value(new std::vector<JsonNode>(value)) {}
            JsonNode(std::initializer_list<JsonNode> value): object_value(value) {}

            // key, value
            JsonNode(std::string key, bool value): key(key), boolean_value(new bool(value)) {}
            JsonNode(std::string key, jstring value): key(key), string_value(new jstring(value)) {}
            JsonNode(std::string key, double value): key(key), number_value(new double(value)) {}
            JsonNode(std::string key, std::vector<JsonNode> value): key(key), vector_value(new std::vector<JsonNode>(value)) {}
            JsonNode(std::string key, std::initializer_list<JsonNode> value): key(key), object_value(value) {}

            ~JsonNode() {
                if (boolean_value != nullptr) {
                    delete boolean_value;
                }
                if (string_value != nullptr) {
                    delete string_value;
                }
                if (number_value != nullptr) {
                    delete number_value;
                }
                if (vector_value != nullptr) {
                    delete vector_value;
                }
            }
    };
}
#endif