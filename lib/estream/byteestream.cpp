#include "estream/byteestream.h"

#include "string_format.h"

#include "string_utils.h"
#include "global_consts.h"

using namespace WylesLibs;

LoopCriteriaState ByteIsCharClassCriteria::untilMatchNext(uint8_t& c) {
    // # decartes
    if (this->char_class & DIGIT_CLASS) {
        this->loop_criteria_info.state = isDigit(c) ? LOOP_CRITERIA_STATE_GOOD : LOOP_CRITERIA_STATE_NOT_GOOD;
    } else if (this->char_class & UPPER_HEX_CLASS) {
        this->loop_criteria_info.state = isUpperHex(c) ? LOOP_CRITERIA_STATE_GOOD : LOOP_CRITERIA_STATE_NOT_GOOD;
    } else if (this->char_class & LOWER_HEX_CLASS) {
        this->loop_criteria_info.state = isLowerHex(c) ? LOOP_CRITERIA_STATE_GOOD : LOOP_CRITERIA_STATE_NOT_GOOD;
    } else if (this->char_class & HEX_CLASS) {
        this->loop_criteria_info.state = isHexDigit(c) ? LOOP_CRITERIA_STATE_GOOD : LOOP_CRITERIA_STATE_NOT_GOOD;
    } else if (this->char_class & ALPHANUMERIC_CLASS) {
        this->loop_criteria_info.state = isAlpha(c) ? LOOP_CRITERIA_STATE_GOOD : LOOP_CRITERIA_STATE_NOT_GOOD;
    }
    return this->loop_criteria_info.state;
}
LoopCriteriaState ByteIsCharClassCriteria::nextState(uint8_t& c) {
    if (LOOP_CRITERIA_UNTIL_MATCH == this->loop_criteria_info.mode) {
        if (++this->loop_criteria_info.until_size > ARBITRARY_SPATIAL_UNTIL_LIMIT) {
            std::string msg = WylesLibs::format("Spatial until limit of {u} reached.", ARBITRARY_SPATIAL_UNTIL_LIMIT);
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
        return this->untilMatchNext(c);
    } else {
        throw std::runtime_error("The match until size good function is not supported for this criteria class.");
    }
}

void ByteStringCollector::initialize() {
    this->data = "";
}

void ByteStringCollector::accumulate(uint8_t& c) {
    this->data.push_back((char)c);
}

void ByteStringCollector::accumulate(SharedArray<uint8_t>& cs) {
    this->data.append((char *)cs.begin(), cs.size());
}

std::string ByteStringCollector::collect() {
    return this->data;
}

void NaturalCollector::initialize() {
    this->digit_count = 0;
    this->value = 0;
}

void NaturalCollector::accumulate(uint8_t& c) {
    this->value = (this->value * 10) + (c - 0x30); 
    // loggerPrintf(LOGGER_DEBUG, "char: 0x%X, value: %lu\n", c, this->value);
    if (++digit_count > NUMBER_MAX_DIGITS) {
        std::string msg = "parseNatural: Exceeded natural digit limit.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
NaturalTuple NaturalCollector::collect() {
    // TODO: implement tuple? also is that template args I see of different types? or is it vaargs?
    //  lol, that would be lame and might be worth reimplementing format? 
    return std::make_tuple(this->value, this->digit_count);
}

void DecimalCollector::initialize() {
    this->decimal_divisor = 10.0;
    this->digit_count = 0;
    this->value = 0;
}

void DecimalCollector::accumulate(uint8_t& c) {
    this->value += (c - 0x30) / this->decimal_divisor;
    loggerPrintf(LOGGER_DEBUG, "char: 0x%X, value: %f\n", c, this->value);
    this->decimal_divisor *= 10;
    if (++digit_count > NUMBER_MAX_DIGITS) {
        std::string msg = "parseDecimal: Exceeded decimal digit limit.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

DecimalTuple DecimalCollector::collect() {
    return std::make_tuple(this->value, this->digit_count, 0);
}

// ( ͡° ͜ʖ ͡°) U+1F608 U+1FAF5
// TODO: tests with operation - it's not completely functional... look into typedef, type coersion and casting.
//      other variations of read should implement a specialization - consumers of child classes can subsequently cast and invoke the specialization.
template<>
NaturalTuple ByteEStream::read<NaturalTuple>(std::string until, StreamTask<uint8_t, NaturalTuple> * operation, bool consume) {
    NaturalTuple t;
    if (until == "") { // default
        ByteIsCharClassCriteria char_class_criteria(ByteIsCharClassCriteria::DIGIT_CLASS);

        t = EStream<uint8_t>::streamCollect<NaturalTuple>(dynamic_cast<LoopCriteria<uint8_t> *>(&char_class_criteria), operation, this->natural_collector);
    } else {
        bool inclusive = false;
        size_t until_size = 0;
        LoopCriteria<uint8_t> until_size_criteria(LoopCriteriaInfo<uint8_t>(LOOP_CRITERIA_UNTIL_MATCH, inclusive, until_size, until));

        t = EStream<uint8_t>::streamCollect<NaturalTuple>(&until_size_criteria, operation, this->natural_collector);
        // ! IMPORTANT - Because collector might not have a way of distinguishing between until char and regular. 
        if (true == consume) {
            this->get(); // consume until char
        }
    }
    // loggerExec(LOGGER_DEBUG_VERBOSE,
    //     if (true == this->good()) {
    //         loggerPrintf(LOGGER_DEBUG_VERBOSE, "Parsed natural, stream is at '%c', [0x%02X]\n", this->peek(), this->peek());
    //     }
    // );
    return t;
}

// ( ͡° ͜ʖ ͡°) U+1F608 U+1FAF5
template<>
NaturalTuple ByteEStream::read<NaturalTuple>(size_t n, StreamTask<uint8_t, NaturalTuple> * operation) {
    if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
        throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
    }
    NaturalTuple t;
    if (n == 0) { // default
        ByteIsCharClassCriteria char_class_criteria(ByteIsCharClassCriteria::DIGIT_CLASS);

        t = EStream<uint8_t>::streamCollect<NaturalTuple>(dynamic_cast<LoopCriteria<uint8_t> *>(&char_class_criteria), operation, this->natural_collector);
    } else {
        bool inclusive = true;
        SharedArray<uint8_t> until;
        LoopCriteria<uint8_t> until_size_criteria(LoopCriteriaInfo<uint8_t>(LOOP_CRITERIA_UNTIL_NUM_ELEMENTS, inclusive, n, SharedArray<uint8_t>(until)));

        t = EStream<uint8_t>::streamCollect<NaturalTuple>(&until_size_criteria, operation, this->natural_collector);
    }
    loggerExec(LOGGER_DEBUG_VERBOSE,
        if (true == this->good()) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Parsed natural, stream is at '%c', [0x%02X]\n", this->peek(), this->peek());
        }
    );
    return t;
}

// ( ͡° ͜ʖ ͡°) U+1F608 U+1FAF5
template<>
DecimalTuple ByteEStream::read<DecimalTuple>(std::string until, StreamTask<uint8_t, DecimalTuple> * operation, bool consume) {
    NaturalTuple natural_value = this->read<NaturalTuple>("", nullptr, false);

    char c = this->peek();
    if (c != '.') {
        if (until != "" && true == consume) {
            this->get(); // consume until char
        }
        return std::make_tuple(static_cast<double>(std::get<0>(natural_value)), std::get<1>(natural_value), 0);
    } else {
        this->get(); // consume .
    }
    DecimalTuple decimal_value;
    if (until == "") { // default
        ByteIsCharClassCriteria char_class_criteria(ByteIsCharClassCriteria::DIGIT_CLASS);

        decimal_value = EStream<uint8_t>::streamCollect<DecimalTuple>(&char_class_criteria, operation, this->decimal_collector);
    } else {
        bool inclusive = false;
        size_t until_size = 0;
        LoopCriteria<uint8_t> until_size_criteria(LoopCriteriaInfo<uint8_t>(LOOP_CRITERIA_UNTIL_MATCH, inclusive, until_size, until));

        decimal_value = EStream<uint8_t>::streamCollect<DecimalTuple>(&until_size_criteria, operation, this->decimal_collector);
        // ! IMPORTANT - Because collector might not have a way of distinguishing between until char and regular? is digit check to costly? 
        //      if not digit then exception? I think that was the question I didn't care to further think about... whatever...
        if (true == consume) {
            this->get(); // consume until char
        }
    }
    loggerExec(LOGGER_DEBUG_VERBOSE,
        if (true == this->good()) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Parsed decimal, stream is at '%c', [0x%02X]\n", this->peek(), this->peek());
        }
    );
    return std::make_tuple(static_cast<double>(std::get<0>(natural_value)) + std::get<0>(decimal_value), std::get<1>(natural_value), std::get<1>(decimal_value));
}

// ( ͡° ͜ʖ ͡°) U+1F608 U+1FAF5
template<>
DecimalTuple ByteEStream::read<DecimalTuple>(size_t n, StreamTask<uint8_t, DecimalTuple> * operation) {
    if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
        throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
    }
    NaturalTuple natural_value = this->read<NaturalTuple>("", nullptr, false);

    size_t num_natural_digits = std::get<1>(natural_value);
    if (num_natural_digits >= n || this->peek() != '.') {
        return std::make_tuple(static_cast<double>(std::get<0>(natural_value)), num_natural_digits, 0);
    } else {
        // num_natural_digits != n && this->peek() == '.'
        this->get(); // consume .
    }
 
    size_t num_to_read = n - num_natural_digits - 1;
    DecimalTuple decimal_value;
    if (num_to_read > 0) { // default
        bool inclusive = true;
        SharedArray<uint8_t> until;
        LoopCriteria<uint8_t> until_size_criteria(LoopCriteriaInfo<uint8_t>(LOOP_CRITERIA_UNTIL_NUM_ELEMENTS, inclusive, n, SharedArray<uint8_t>(until)));

        decimal_value = EStream<uint8_t>::streamCollect<DecimalTuple>(&until_size_criteria, operation, this->decimal_collector);
    }
    loggerExec(LOGGER_DEBUG_VERBOSE,
        if (true == this->good()) {
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Parsed decimal, stream is at '%c', [0x%02X]\n", this->peek(), this->peek());
        }
    );
    return std::make_tuple(static_cast<double>(std::get<0>(natural_value)) + std::get<0>(decimal_value), num_natural_digits, std::get<1>(decimal_value));
}

#ifdef WYLESLIBS_SSL_ENABLED
void SSLEStream::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = SSL_read(this->ssl, this->buffer, this->buffer_size * sizeof(uint8_t)); // TODO: sizeof(uint8_t) == 1;
    // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
    if (ret <= 0 || (size_t)ret > this->buffer_size * sizeof(uint8_t)) {
        this->els_in_buffer = 0;
        loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
        this->flags |= std::ios_base::badbit;
        throw std::runtime_error("Read error.");
    } else {
        this->els_in_buffer = ret / sizeof(uint8_t);
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld els from tls layer.\n", ret);
    }
}

ssize_t SSLEStream::write(uint8_t * b, size_t size) {
    return SSL_write(this->ssl, (void *)b, size);
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

        // ! IMPORTANT - apparently this is optional. SSL_read will implicitly perform handshake but that doesn't seem like the correct approach.
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