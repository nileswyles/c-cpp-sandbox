#include "reader.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef LOGGER_READER
#define LOGGER_READER 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_READER
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
    operation->read_until = until;

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

    // make this make sense....
    printf("BACK| %c\n", data.buf[data.size()-2]);
    return data;
}

void Reader::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = read(this->r_fd, this->buf, this->buf_size);
    // TODO: retry on EAGAIN?, revisit possible errors...
    if (ret == -1 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        // second should never happen but if we're being thorough...
        loggerPrintf(LOGGER_ERROR, "Read error: %d\n", errno);
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
    }
}

void Reader::cursorCheck() {
    if (this->cursor >= this->bytes_in_buffer) {
        fillBuffer();
    }
}

// this is still an arbitrary limit chosen based on number of digits of 2**32.
//  it also satisfies requirement of single precision significand size (2**24) which is well within 10 digits (10**10 - 1).
static constexpr size_t FLT_MAX_MIN_DIGITS = 10;

void Reader::readDecimal(double& value, size_t& digit_count) {
    double decimal_divisor = 10;
    char c = this->peekByte();
    while (isDigit(c)) {
        this->readByte();
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = this->peekByte();
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
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
        if (++digit_count > FLT_MAX_MIN_DIGITS) {
            std::string msg = "parseNatural: Exceeded natural digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}