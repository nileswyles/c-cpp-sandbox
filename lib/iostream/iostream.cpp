#include "iostream.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "global_consts.h"

#define ARBITRARY_LIMIT_BECAUSE_DUMB 4294967296

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_IOSTREAM
#define LOGGER_LEVEL_IOSTREAM GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_IOSTREAM
#define LOGGER_IOSTREAM 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_IOSTREAM

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_IOSTREAM
#include "logger.h"

using namespace WylesLibs;

uint8_t IOStream::peekByte() {
    this->cursorCheck();
    return this->buf[this->cursor];
}

uint8_t IOStream::readByte() {
    uint8_t byte = peekByte();
    this->cursor++;
    return byte;
}

ssize_t IOStream::writeBuffer(void * p_buf, size_t size) {
#ifdef WYLESLIBS_SSL_ENABLED
    if (this->ssl == nullptr) {
        return write(this->fd, p_buf, size);
    } else {
        return SSL_write(this->ssl, p_buf, size);
    }
#else 
    return write(this->fd, p_buf, size);
#endif
}

SharedArray<uint8_t> IOStream::readBytes(const size_t n) {
    this->cursorCheck();

    if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
        throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
    }

    SharedArray<uint8_t> data;
    size_t bytes_read = 0;
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

SharedArray<uint8_t> IOStream::readUntil(std::string until, ReaderTask * operation, bool inclusive) {
    // # less clunky
    if (operation != nullptr) {
        operation->read_until = until;
    }

    this->cursorCheck();

    SharedArray<uint8_t> data;
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

void IOStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = -1;
#ifdef WYLESLIBS_SSL_ENABLED
    if (this->ssl == nullptr) {
        ret = read(this->fd, this->buf, this->buf_size);
    } else {
        ret = SSL_read(this->ssl, this->buf, this->buf_size);
    }
#else 
    ret = read(this->fd, this->buf, this->buf_size);
#endif
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        loggerPrintf(LOGGER_ERROR, "Read error: %d, ret: %ld\n", errno, ret);
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
        loggerExec(LOGGER_DEBUG_VERBOSE,
#ifdef WYLESLIBS_SSL_ENABLED
            if (this->ssl == nullptr) {
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from transport layer.\n", ret);
            } else {
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from tls layer.\n", ret);
            }
#else 
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from transport layer.\n", ret);
#endif
        );
    }
}

void IOStream::cursorCheck() {
    if (this->cursor >= this->bytes_in_buffer) {
        fillBuffer();
    }
}

// TODO: 
//  streamify, these as well.
//  also, think about what other generalizations can be made - think map/reduce/filter/collect
//  some of the afforementioned operations are already accounted for.
void IOStream::readDecimal(double& value, size_t& digit_count) {
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

void IOStream::readNatural(double& value, size_t& digit_count) {
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