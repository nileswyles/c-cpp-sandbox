#include "estream/byteestream.h"

#include "string_utils.h"
#include "global_consts.h"

using namespace WylesLibs;

bool ByteIsCharClassCriteria::untilMatchGood(uint8_t& c) {
    bool good = false;
    if (this->char_class & DIGIT_CLASS) {
        good = isDigit(c);
        printf("isDigit(): %u, char: %c\n", good, c);
    } else if (this->char_class & UPPER_HEX_CLASS) {
        good = isUpperHex(c);
    } else if (this->char_class & LOWER_HEX_CLASS) {
        good = isLowerHex(c);
    } else if (this->char_class & HEX_CLASS) {
        good = isHexDigit(c);
    } else if (this->char_class & ALPHANUMERIC_CLASS) {
        good = isAlpha(c);
    }
    // } else if (char_class == SPECIAL) {
    return good;
}
bool ByteIsCharClassCriteria::good(uint8_t& c) {
    if (LOOP_CRITERIA_UNTIL_MATCH == mode) {
        return this->untilMatchGood(c);
    } else {
        throw std::runtime_error("The match until size good function is not supported for this criteria class.");
    }
}

void ByteCollector::accumulate(uint8_t& c) {
    this->data.append(c);
}
void ByteCollector::accumulate(SharedArray<uint8_t>& cs) {
    this->data.append(cs);
}
SharedArray<uint8_t> ByteCollector::collect() {
    return this->data;
}
template<>
std::shared_ptr<Collector<uint8_t, SharedArray<uint8_t>>> WylesLibs::initReadCollector<uint8_t, SharedArray<uint8_t>>() {
    return std::dynamic_pointer_cast<Collector<uint8_t, SharedArray<uint8_t>>>(std::make_shared<ByteCollector>());
}

void NaturalCollector::accumulate(uint8_t& c) {
    // 0 * 10 + (0) = 0
    // (0 * 10) + (3)
    value = (value * 10) + (c - 0x30); 
    loggerPrintf(LOGGER_DEBUG, "char: 0x%X, value: %lu\n", c, value);
    if (++digit_count > NUMBER_MAX_DIGITS) {
        std::string msg = "parseNatural: Exceeded natural digit limit.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
uint64_t NaturalCollector::collect() {
    uint64_t v = this->value;
    this->value = 0;
    return v;
}

void DecimalCollector::accumulate(uint8_t& c) {
    value += (c - 0x30) / decimal_divisor;
    loggerPrintf(LOGGER_DEBUG, "value: %f\n", value);
    decimal_divisor *= 10;
    if (++digit_count > NUMBER_MAX_DIGITS) {
        std::string msg = "parseDecimal: Exceeded decimal digit limit.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
double DecimalCollector::collect() {
    double v = this->value;
    this->value = 0.0;
    return v;
}

SharedArray<uint8_t> ByteEStream::read(std::string until, ReaderTask * operation, bool inclusive) {
    // TODO: no overloading but yes to explict disambiguising? lmao
    SharedArray<uint8_t> until_arr(until);
    printf("until: '%s', until_arr: '%s'\n", until.c_str(), until_arr.toString().c_str());
    return EStream<uint8_t>::read(until_arr, operation, inclusive);
}

uint64_t ByteEStream::readNatural() {
    ByteIsCharClassCriteria * criteria = dynamic_cast<ByteIsCharClassCriteria *>(this->natural_processor.criteria.get());
    criteria->mode = LOOP_CRITERIA_UNTIL_MATCH;
    criteria->char_class = ByteIsCharClassCriteria::DIGIT_CLASS;

    return this->natural_processor.streamCollect(this, nullptr);
}

double ByteEStream::readDecimal() {
    ByteIsCharClassCriteria * criteria = dynamic_cast<ByteIsCharClassCriteria *>(this->natural_processor.criteria.get());
    criteria->mode = LOOP_CRITERIA_UNTIL_MATCH;
    criteria->char_class = ByteIsCharClassCriteria::DIGIT_CLASS;

    double natural_value = static_cast<double>(this->natural_processor.streamCollect(this, nullptr));

    this->unget();
    char c = this->get();
    if (c != '.') {
        std::string msg = "Invalid number.";
        loggerPrintf(LOGGER_INFO, "%s, found '%c'\n", msg.c_str(), c);
        throw std::runtime_error(msg);
    }

    criteria = dynamic_cast<ByteIsCharClassCriteria *>(this->decimal_processor.criteria.get());
    criteria->mode = LOOP_CRITERIA_UNTIL_MATCH;
    criteria->char_class = ByteIsCharClassCriteria::DIGIT_CLASS;
    return this->decimal_processor.streamCollect(this, nullptr) + natural_value;
}


#ifdef WYLESLIBS_SSL_ENABLED
void SSLEStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = SSL_read(this->ssl, this->buf, this->buf_size * sizeof(uint8_t)); // TODO: sizeof(uint8_t) == 1;
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buf_size) {
        this->els_in_buffer = 0;
        loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
        this->flags |= std::ios_base::badbit;
        throw std::runtime_error("Read error.");
    } else {
        this->els_in_buffer = ret;
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld els from tls layer.\n", ret);
    }
}

ssize_t SSLEStream::write(uint8_t * p_buf, size_t size) {
    return SSL_write(this->ssl, (void *)p_buf, size);
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
    #endif