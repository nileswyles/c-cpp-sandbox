#include "estream/istreamestream.h"

#define MAX_STREAM_LOG_SIZE 255
#define ARBITRARY_LIMIT_BECAUSE_DUMB 4294967296

using namespace WylesLibs;

bool IStreamEStream::readPastBuffer() {
    return ESHAREDPTR_GET_PTR(this->reader)->good();
}

void IStreamEStream::fillBuffer() {
    // get new stream from underlying transport...
    if (!this->factory) {
        throw std::runtime_error("Read error.");
    } else {
        this->file_offset += this->chunk_size;
        reader = ESHAREDPTR_GET_PTR(this->factory)->reader(path, this->file_offset, this->chunk_size);
    }
}
uint8_t IStreamEStream::get() {
    if (true == this->readPastBuffer()) {
        this->fillBuffer();
    }
    uint8_t c = ESHAREDPTR_GET_PTR(this->reader)->get();
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
    this->stream_log += c;
    if (this->stream_log.size() > MAX_STREAM_LOG_SIZE) {
        this->stream_log.clear();
    }
#endif
    return c;
}

void IStreamEStream::unget() {
    ESHAREDPTR_GET_PTR(this->reader)->unget();
}

uint8_t IStreamEStream::peek() {
    if (true == this->readPastBuffer()) {
        this->fillBuffer();
    }
    return ESHAREDPTR_GET_PTR(this->reader)->peek();
}

bool IStreamEStream::eof() {
    return ESHAREDPTR_GET_PTR(this->reader)->eof();
}

bool IStreamEStream::good() {
    return ESHAREDPTR_GET_PTR(this->reader)->good();
}

bool IStreamEStream::fail() {
    return ESHAREDPTR_GET_PTR(this->reader)->fail();
}

// TODO: this isn't very useful in the current state.
void IStreamEStream::seekg(size_t offset) {
    ESHAREDPTR_GET_PTR(this->reader)->seekg(offset);
}

SharedArray<uint8_t> IStreamEStream::read(const size_t n, StreamTask<uint8_t, SharedArray<uint8_t>> * operation) {
    if (n == 0) {
        throw std::runtime_error("It doesn't make sense to read zero els.");
    } else if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
        throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
    }
    if (operation == nullptr) {
        SharedArray<uint8_t> data;
        for (size_t i = 0; i < n; i++) {
            data.append(this->get());
        }
        return data;
    } else {
        return this->read(n, operation);
    }
}
ssize_t IStreamEStream::write(uint8_t * p_buf, size_t size) {
    throw std::runtime_error("This function is not available for this extension of ByteEStream base.");
}