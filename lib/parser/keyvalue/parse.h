#ifndef WYLESLIBS_KV_PARSE_H
#define WYLESLIBS_KV_PARSE_H

#include "estream/byteestream.h"

#include <string>
#include <unordered_map>
#include <stdexcept>

#ifndef LOGGER_JSON_PARSER
#define LOGGER_JSON_PARSER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_PARSER
#include "logger.h"

#define MAX_LENGTH_OF_KEYVALUE_STRING 1024

namespace WylesLibs::Parser::KeyValue {
    // TODO: refactor this and usages to regular map 
    extern std::string toString(std::unordered_map<std::string, std::string> map, char key_delim, char delim);
    
    extern std::unordered_map<std::string, std::string> parse(std::string s, char key_delim, char delim);
    // ! IMPORTANT - Newline after kv string is no longer required.
    extern std::unordered_map<std::string, std::string> parse(ByteEStream * reader, char key_delim, char delim);
    
    // LOL
    static std::unordered_map<std::string, std::string> parse(std::string s, char delim) {
        return parse(s, '=', delim);
    }
    static std::unordered_map<std::string, std::string> parse(ByteEStream * reader, char delim) {
        return parse(reader, '=', delim);
    }
}

#endif