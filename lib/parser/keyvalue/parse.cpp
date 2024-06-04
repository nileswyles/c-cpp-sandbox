#include "parser/keyvalue/parse.h"

std::unordered_map<std::string, std::string> WylesLibs::Parser::KeyValue::parse(std::string s, char delim) {
    loggerPrintf(LOGGER_DEBUG, "JSON: \n");
    loggerPrintf(LOGGER_DEBUG, "%s\n", s.c_str());
    if (s.size() > MAX_LENGTH_OF_KEYVALUE_STRING) {
        std::string msg = "String to loooonnnng!";
        loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    Reader r((uint8_t *)s.data(), s.size());
    return parse(&r, delim);
}

// ! IMPORTANT - Newline after kv string is required.
std::unordered_map<std::string, std::string> WylesLibs::Parser::KeyValue::parse(Reader * reader, char delim) {
    std::unordered_map<std::string, std::string> data;

    std::string delim_string = "\n";
    if (delim != '\n') {
        delim_string += delim;
    }
    while (1) {
        //  TODO: allow specifying
        //      key/value delim
        //      =, :, etc...

        //      also, trim whitespace?
        Array<uint8_t> field_name = reader->readUntil("=\n");
        if (field_name.back() == '\n') {
            // empty line... we are done...
            break;
        }
        Array<uint8_t> field_value = reader->readUntil(delim_string);
        if (delim != '\n' && field_value.back() == '\n') {
            // new line instead of delim, so we are done.
            break;
        }
        data[field_name.popBack().toString()] = field_value.popBack().toString();
    }
    return data;
} 