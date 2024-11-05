#include "estream.h"

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

void ReaderEStream::cursorCheck() {
    if (false == this->reader->good()) {
        this->fillBuffer();
    }
}

void EStream::cursorCheck() {
    if (this->cursor >= this->bytes_in_buffer) {
        this->fillBuffer();
    }
}

void ReaderEStream::fillBuffer() {
    // get new stream from underlying transport...
    if (this->factory == nullptr) {
        throw std::runtime_error("Read error.");
    } else {
        this->file_offset += this->chunk_size;
        reader = this->factory->reader(path, this->file_offset, this->chunk_size);
    }
}

void EStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = ::read(this->fd, this->buf, this->buf_size);
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        loggerPrintf(LOGGER_ERROR, "Read error: %d, ret: %ld\n", errno, ret);
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
        loggerExec(LOGGER_DEBUG_VERBOSE,
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from transport layer.\n", ret);
        );
    }
}

#ifdef WYLESLIBS_SSL_ENABLED
void SSLEStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = SSL_read(this->ssl, this->buf, this->buf_size);
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        loggerPrintf(LOGGER_ERROR, "Read error: %d, ret: %ld\n", errno, ret);
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
#endif

uint8_t ReaderEStream::get() {
    this->cursorCheck();
    return this->reader->get();
}
uint8_t ReaderEStream::peek() {
    this->cursorCheck();
    return this->reader->peek();
}
bool ReaderEStream::eof() {
    return this->reader->eof();
}
bool ReaderEStream::good() {
    return this->reader->good();
}
bool ReaderEStream::fail() {
    return this->reader->fail();
}
void ReaderEStream::seekg(size_t offset) {
    this->reader->seekg(offset);
}

uint8_t EStream::get() {
    char byte = this->peek();
    this->cursor++;
    return byte;
}
uint8_t EStream::peek() {
    this->cursorCheck();
    return this->buf[this->cursor];
}
// Stub these out for now.
bool EStream::eof() {
    return false;
}
bool EStream::good() {
    return true;
}
bool EStream::fail() {
    return false;
}
// char_type EStream::read() override;
SharedArray<uint8_t> ReaderEStream::readBytes(const size_t n) {
    // yuck
    // TODO: casting is annoying...
    return SharedArray<uint8_t>(std::reinterpret_pointer_cast<std::basic_istream<uint8_t>>(this->reader), n);
}

SharedArray<uint8_t> EStream::readBytes(const size_t n) {
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

SharedArray<uint8_t> ReaderEStream::readUntil(std::string until, ReaderTask * operation, bool inclusive) {
    // # less clunky
    if (operation != nullptr) {
        operation->read_until = until;
    }

    SharedArray<uint8_t> data;
    uint8_t c = this->peek();
    while (until.find(c) == std::string::npos) {
        this->get(); // consume
        // commit character
        if (operation != nullptr) {
            operation->perform(data, c);
        } else {
            data.append(c);
        }
        c = this->peek();
    }
    if (inclusive) {
        // commit character
        if (operation != nullptr) {
            operation->perform(data, c);
        } else {
            data.append(c);
        }
        // make sure to move cursor past until char. else still point to until char for next read...
        this->get();
    }
    if (operation != nullptr) {
        operation->flush(data);
    }

    loggerPrintf(LOGGER_DEBUG, "reader_read_until string: '%s'\n", data.toString().c_str());

    return data;
}

// TODO: 
//  streamify, these as well.
//  also, think about what other generalizations can be made - think map/reduce/filter/collect
//  some of the afforementioned operations are already accounted for.
void ReaderEStream::readDecimal(double& value, size_t& digit_count) {
    double decimal_divisor = 10;
    char c = this->peek();
    while (isDigit(c)) {
        this->get();
        value += (c - 0x30) / decimal_divisor;
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        decimal_divisor *= 10;
        c = this->peek();
        if (++digit_count > NUMBER_MAX_DIGITS) {
            std::string msg = "parseDecimal: Exceeded decimal digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}

void ReaderEStream::readNatural(double& value, size_t& digit_count) {
    char c = this->peek();
    while (isDigit(c)) {
        this->get();
        value = (value * 10) + (c - 0x30); 
        loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
        c = this->peek();
        if (++digit_count > NUMBER_MAX_DIGITS) {
            std::string msg = "parseNatural: Exceeded natural digit limit.";
            loggerPrintf(LOGGER_ERROR, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
    }
}

ssize_t EStream::write(void * p_buf, size_t size) {
    return ::write(this->fd, p_buf, size);
}

#ifdef WYLESLIBS_SSL_ENABLED
ssize_t SSLEStream::write(void * p_buf, size_t size) {
    return SSL_write(this->ssl, p_buf, size);
}
#endif