#ifndef WYLESLIBS_KV_PARSE_H
#define WYLESLIBS_KV_PARSE_H

#include "reader/reader.h"

#include <string>
#include <map>
#include <stdexcept>

#ifndef LOGGER_JSON_PARSER
#define LOGGER_JSON_PARSER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_JSON_PARSER
#include "logger.h"

#define MAX_LENGTH_OF_KEYVALUE_STRING 1024

namespace WylesLibs::Parser::KeyValue {

extern map<std::string, std::string> parse(std::string s, char delim);
// ! IMPORTANT - Newline after kv string is required.
extern map<std::string, std::string> parse(Reader * reader, char delim);

}

#endif