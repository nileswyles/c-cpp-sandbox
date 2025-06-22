#ifndef WYLESLIBS_JSON_PARSER_H
#define WYLESLIBS_JSON_PARSER_H

#include <map>
#include <string>
#include <tuple>
#include <filesystem>

#include <stdint.h>
#include <stdbool.h>

#include "memory/pointers.h"
#include "datastructures/array.h"
#include "estream/byteestream.h"
#include "file/file.h"
#include "string_utils.h"
#include "string_format.h"

#include "parser/json/json_mix.h"
#include "parser/json/json_object.h"
#include "parser/json/json_array.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

namespace WylesLibs::Parser::Json {
    // @

    extern ESharedPtr<JsonValue> parseFile(ESharedPtr<StreamFactory> stream_factory, std::string file_path, bool exceptions = false);
    extern ESharedPtr<JsonValue> parse(std::string json);
    extern ESharedPtr<JsonValue> parse(SharedArray<uint8_t> json);
    extern ESharedPtr<JsonValue> parse(ByteEStream * r, size_t& i);

    extern std::string pretty(std::string json);

    // ! IMPORTANT - T must implement the JsonBase interface. Useful when you don't want to implement an entire Json class.
    template<typename T>
    extern std::string toString(std::map<std::string, T> r, size_t depth = 0);
}
#endif