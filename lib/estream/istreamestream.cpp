#include "estream/istreamestream.h"

#define MAX_STREAM_LOG_SIZE 255
#define ARBITRARY_LIMIT_BECAUSE_DUMB 4294967296

using namespace WylesLibs;

bool IStreamEStream::readPastBuffer() {
    // TODO: this might not be correct for all cases.
    //      review eof, good, fail flags again.
    return false == this->good();
}

void IStreamEStream::fillBuffer() {
    // get new stream from underlying transport...
    this->file_offset += this->chunk_size;
    reader = ESHAREDPTR_GET_PTR(this->factory)->reader(path, this->file_offset, this->chunk_size);
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

ssize_t IStreamEStream::write(uint8_t * b, size_t size) {
    throw std::runtime_error("This function is not available for this extension of ByteEStream base.");
}