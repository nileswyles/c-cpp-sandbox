#include "parser/keyvalue/parse.h"

std::unordered_map<std::string, std::string> WylesLibs::Parser::KeyValue::parse(std::string s, char key_delim, char delim) {
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", s.c_str());
    if (s.size() > MAX_LENGTH_OF_KEYVALUE_STRING) {
        std::string msg = "String to loooonnnng!";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    ByteEStream r((uint8_t *)s.data(), s.size());
    return parse(&r, key_delim, delim);
}

// ! IMPORTANT - Newline after kv string is required.
std::unordered_map<std::string, std::string> WylesLibs::Parser::KeyValue::parse(ByteEStream * reader, char key_delim, char delim) {
    std::unordered_map<std::string, std::string> data;

    std::string delim_string = "\n";
    if (delim != '\n') {
        delim_string += delim;
    }
    while (1) {
        ReaderTaskDisallow<SharedArray<uint8_t>> ignore_whitespace(" \t");
        SharedArray<uint8_t> field_name = reader->read("=\n", &ignore_whitespace);
        if (field_name.back() == '\n') {
            // empty line... we are done...
            break;
        }
        SharedArray<uint8_t> field_value = reader->read(delim_string);
        if (delim != '\n' && field_value.back() == '\n') {
            // new line instead of delim, so we are done.
            break;
        }
        data[field_name.removeBack().toString()] = field_value.removeBack().toString();
    }
    return data;
} 