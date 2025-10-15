#ifndef WYLESLIBS_ESTREAM_IO_H
#define WYLESLIBS_ESTREAM_IO_H

#include "estream/istreamestream.h"
#include "datastructures/array.h"
#include "memory/pointers.h"

#include <ios>
#include <string>

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_IO
#define LOGGER_LEVEL_IO GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_IO
#define LOGGER_IO 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_IO

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_IO
#include "logger.h"

namespace WylesLibs {

template<typename T>
static void write(ESharedPtr<std::basic_ostream<char>> s_shared, T buffer, bool append = false) {
    std::basic_ostream<char> * s = ESHAREDPTR_GET_PTR(s_shared);

    if (true == append) {
        s->seekp(0, std::basic_ostream<char>::end);
    } else {
        // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
        s->seekp(0);
    }
    s->write((char *)buffer.data(), buffer.size()); // binary output
    s->flush();
}

template<typename T>
static void write(ESharedPtr<std::basic_ostream<char>> s_shared, T buffer, size_t offset = 0) {
    std::basic_ostream<char> * s = ESHAREDPTR_GET_PTR(s_shared);

    s->seekp(offset);
    s->write((const char *)buffer.data(), buffer.size()); // binary output
    s->flush();
}

static SharedArray<uint8_t> read(ESharedPtr<IStreamEStream> s_shared, size_t offset = 0, size_t size = SIZE_MAX) {
    IStreamEStream * s = ESHAREDPTR_GET_PTR(s_shared);

    SharedArray<uint8_t> file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (true == s->good()) { //  && false == s->eof(); implied
            uint8_t c = s->get();
            // good and eof are true, false (respectively) at eof character, so ignore.
            if (c != 0xFF) {
                file_data.append(c);
            }
        }
        // if (true == s->fail()) {
        //     throw std::runtime_error("Error occured while reading istream until EOF.");
        // }
    } else {
        file_data = s->readEls(size);
    }
    return file_data;
}

// TODO: refactored to use string type until I can better architect the EStream stuff... need better templating support
static std::string readString(ESharedPtr<IStreamEStream> s_shared, size_t offset = 0, size_t size = SIZE_MAX) {
    IStreamEStream * s = ESHAREDPTR_GET_PTR(s_shared);

    std::string file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (true == s->good()) { //  && false == s->eof(); implied
            uint8_t c = s->get();
            // good and eof are true, false (respectively) at eof character, so ignore.
            if (c != 0xFF) {
                file_data += c;
            }
        }
        // if (true == s->fail()) {
        //     throw std::runtime_error("Error occured while reading istream until EOF.");
        // }
    } else {
        file_data = s->readString(size);
    }
    return file_data;
}

};

#endif