#include "reader.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "global_consts.h"

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_READER
#define LOGGER_LEVEL_READER GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_READER
#define LOGGER_READER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_READER

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_READER
#include "logger.h"

using namespace WylesLibs;

uint8_t Reader::peekByte() {
    this->cursorCheck();
    return this->buf[this->cursor];
}

uint8_t Reader::readByte() {
    uint8_t byte = peekByte();
    this->cursor++;
    return byte;
}

Array<uint8_t> Reader::readBytes(const size_t n) {
    this->cursorCheck();

    Array<uint8_t> data;
    size_t bytes_read = 0;
    // TODO: limit size of this->buffer?
    while (bytes_read < n) {
        size_t bytes_left_to_read = n - bytes_read;
        size_t bytes_left_in_buffer = this->bytes_in_buffer - this->cursor;
        if (bytes_left_to_read > bytes_left_in_buffer) {
            // copy data left in buffer and read more
            data.append(this->buf + this->cursor, bytes_left_in_buffer);
            bytes_read += bytes_left_in_buffer;

            fillBuffer();
        } else {
            // else enough data in buffer
            data.append(this->buf + this->cursor, bytes_left_to_read);
            this->cursor += bytes_left_to_read;
            bytes_read += bytes_left_to_read;
        }
    } 
    return data;
}

Array<uint8_t> Reader::readUntil(std::string until, ReaderTask * operation, bool inclusive) {
    // # less clunky
    if (operation != nullptr) {
        operation->read_until = until;
    }

    this->cursorCheck();

    Array<uint8_t> data;
    uint8_t c = this->buf[this->cursor];
    while (until.find(c) == std::string::npos) {
        // commit character
        if (operation != nullptr) {
            operation->perform(data, c);
        } else {
            data.append(c);
        }
        // move to next character
        this->cursor++;
        // if no more data in buffer (cursor beyond end of buffer), fill buffer and reset cursor...
        this->cursorCheck();
        // points to new value...
        c = this->buf[this->cursor]; 
    }
    if (inclusive) {
        // commit character
        if (operation != nullptr) {
            operation->perform(data, c);
        } else {
            data.append(c);
        }
        // make sure to move cursor past until char. else still point to until char for next read...
        this->cursor++;
    }
    if (operation != nullptr) {
        operation->flush(data);
    }

    loggerPrintf(LOGGER_DEBUG, "reader_read_until end cursor: %lu\n", this->cursor);
    loggerPrintf(LOGGER_DEBUG, "reader_read_until string: '%s'\n", data.toString().c_str());

    return data;
}

void Reader::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = -1;
    if (this->ssl == nullptr) {
        ret = read(this->r_fd, this->buf, this->buf_size);
    } else {
        ret = SSL_read(this->ssl, this->buf, this->buf_size);
    }
    // TODO: retry on EAGAIN?, revisit possible errors...
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        loggerPrintf(LOGGER_ERROR, "Read error: %d\n", errno);
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
        loggerExec(LOGGER_DEBUG_VERBOSE,
            if (this->ssl == nullptr) {
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from transport layer.\n", ret);
            } else {
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from tls layer.\n", ret);
            }
        );
    }
}

void Reader::cursorCheck() {
    if (this->cursor >= this->bytes_in_buffer) {
        fillBuffer();
    }
}

// TODO: 
//  streamify, these as well.
//  also, think about what other generalizations can be made - think map/reduce/filter/collect
//  some of the afforementioned operations are already accounted for.
void Reader::readDecimal(double& value, size_t& digit_count) {
    double decimal_divisor = 10;
    char c = this->peekByte();
    while (isDigit(c)) {
        this->readByte();
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = this->peekByte();
        if (++digit_count > NUMBER_MAX_DIGITS) {
            std::string msg = "parseDecimal: Exceeded decimal digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}

void Reader::readNatural(double& value, size_t& digit_count) {
    char c = this->peekByte();
    while (isDigit(c)) {
        this->readByte();
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = this->peekByte();
        if (++digit_count > NUMBER_MAX_DIGITS) {
            std::string msg = "parseNatural: Exceeded natural digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}