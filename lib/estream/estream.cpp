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
#ifndef LOGGER_LEVEL_ESTREAM
#define LOGGER_LEVEL_ESTREAM GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_ESTREAM
#define LOGGER_ESTREAM 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ESTREAM

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_ESTREAM
#include "logger.h"

using namespace WylesLibs;

// iostate for estream...

// readUntil and readBytes read until finish criteria else throws an exception
// get and peek throw exception (when configured as such) if stream->good() == false, whether as a result of this operation or not.

//  iostate is defined as having irrecoverable and recoverable error state
// Ideas:
//  each read operation needs to detect errors and update iostate as needed...
//      okay, yeah so, that's decided. we'll update iostate state if an error detected in fillBuffer..

//  additionally, the good function might want to check the fd again and update the state... if not bad (irrecoverable?)
//      what's an irrecoverable error?
//          eof?
//          EBADF == fd not open for reading..
//          EFAULT == buf unaddressable
//          EISDIR-EINVAL == unreadable fd
//          EIO == 
//          
//      what's a recoverable error?
//          EAGAIN == 
//          EINTR == interrupted before data read
//          POLLIN == 0 but fd still good...
//          if no FD, then good checks whether read until end of buffer...
//  
//      alright, so generally fill buffer will throw an exception and set it as an unrecoverable error...
//      fair enough...

//      recoverable error state is only set by calls to good...
//      
//      alright, sound like a plan...

//  iostate for ReaderEStream currently just returns the state of the has-a stream...
//      let's work through that again with fillBuffer and cursorCheck in mind... just to be sure...
//      you can check underlying stream for iostate ahead of time... 
//      if read call requires more data and has-a stream can't statisfy then get new reader from factory.
//      seems reasonable...

//  okay, so that's reasonable for EStream reading... but write buffer....
//      fd can be valid for reading but not writing? right? or valid for writing but not reading?
//      might not matter for now or in most scenarios, so let's keep them the same...

// while we're at it, let's also implement the bool and ! operators on stream?
bool ReaderEStream::readPastBuffer() {
    return this->reader->good();
}
bool EStream::readPastBuffer() {
    return this->cursor >= this->bytes_in_buffer;
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
        loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
        flags |= std::ios_base::badbit;
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from transport layer.\n", ret);
    }
}
#ifdef WYLESLIBS_SSL_ENABLED
void SSLEStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = SSL_read(this->ssl, this->buf, this->buf_size);
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0;
        loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
        flags |= std::ios_base::badbit;
        throw std::runtime_error("Read error.");
    } else {
        this->bytes_in_buffer = ret;
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld bytes from tls layer.\n", ret);
    }
}
#endif

uint8_t ReaderEStream::get() {
    if (true == this->readPastBuffer()) {
        this->fillBuffer();
    }
    return this->reader->get();
}
uint8_t ReaderEStream::peek() {
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
    if (true == this->good()) {
        if (true == this->readPastBuffer()) {
            this->fillBuffer();
        }
        return this->buf[this->cursor];
    } else {
        // EOF
        return 0xFF;
    }
}
// Stub these out for now.
bool EStream::eof() {
    return flags & std::ios_base::eofbit;
}
bool EStream::good() {
    if (flags == 0) {
        if (true == this->readPastBuffer()) {
            if (this->fd < 0) {
                flags |= std::ios_base::badbit;
            } else {
                if (1 != poll(&this->poll_fd, 1, 0)) {
                    flags |= std::ios_base::badbit;
                }
            }
        }
    }
    return flags == 0;
}
bool EStream::fail() {
    return flags & std::ios_base::failbit;
}
// char_type EStream::read() override;
SharedArray<uint8_t> ReaderEStream::readBytes(const size_t n) {
    // yuck
    // TODO: casting is annoying...
    return SharedArray<uint8_t>(std::reinterpret_pointer_cast<std::basic_istream<uint8_t>>(this->reader), n);
}

SharedArray<uint8_t> EStream::readBytes(const size_t n) {
    if (true == this->readPastBuffer()) {
        this->fillBuffer();
    }

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
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
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
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
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

SSL * SSLEStream::acceptTLS(SSL_CTX * context, int fd, bool client_auth_enabled) {
    if (context == nullptr) {
        throw std::runtime_error("Server SSL Context isn't initialized. Check server configuration.");
    } else {
        SSL * ssl = SSL_new(context);
        if (ssl == nullptr) {
            throw std::runtime_error("Error initializing SSL object for connection.");
        }

        int verify_mode = SSL_VERIFY_NONE;
        if (true == client_auth_enabled) {
            verify_mode = SSL_VERIFY_PEER;
        }
        SSL_set_verify(ssl, verify_mode, nullptr);
        SSL_set_accept_state(ssl);

        SSL_set_fd(ssl, fd);

        SSL_clear_mode(ssl, 0);
        // SSL_MODE_AUTO_RETRY

        // TODO: so apparently this is optional lol... SSL_read will perform handshake...
        //  also, investigate renegotiation, auto retry...
        int accept_result = SSL_accept(ssl);
        loggerPrintf(LOGGER_DEBUG, "ACCEPT RESULT: %d\n", accept_result);
        loggerPrintf(LOGGER_DEBUG, "MODE: %lx, VERSION: %s, IS SERVER: %d\n", SSL_get_mode(ssl), SSL_get_version(ssl), SSL_is_server(ssl));
        loggerExec(LOGGER_DEBUG, SSL_SESSION_print_fp(stdout, SSL_get_session(ssl)););
        if (accept_result != 1) {
            int error_code = SSL_get_error(ssl, accept_result) + 0x30;
            // SSL_ERROR_NONE
            throw std::runtime_error("SSL handshake failed. ERROR CODE: " + std::string((char *)&error_code));
        } // connection accepted if accepted_result == 1

        return ssl;
    }
}
#endif