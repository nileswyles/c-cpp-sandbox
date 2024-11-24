#ifndef WYLESLIBS_ESTREAM_H
#define WYLESLIBS_ESTREAM_H

#include "estream/estream_types.h"

#include "datastructures/array.h"
#include "string_format.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>
#include <memory>
#include "eshared_ptr.h"
#include <ios>
#include <istream>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <poll.h>

#ifdef WYLESLIBS_SSL_ENABLED
#include <openssl/ssl.h>
#endif

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

#define MAX_STREAM_LOG_SIZE 255

#define READER_RECOMMENDED_BUF_SIZE 8096
// ! IMPORTANT - 
//      server.c::MAX_CONNECTIONS is (1 << 16) 64KB... 
//       64k * 4048 == 249072KB or ~256MB
#define READER_RECOMMENDED_BUF_SIZE_SSL 4048

namespace WylesLibs {
template<typename T>
class EStreamI {
    /*
        Read and Write from file descriptor
    */
    protected:
        virtual bool readPastBuffer() = 0;
        virtual void fillBuffer() = 0;
    public:
        EStreamI() = default;
        virtual ~EStreamI() = default;

        // standard istream methods
        virtual T get() = 0;
        virtual T peek() = 0;
        virtual void unget() = 0;
        virtual bool eof() = 0;
        virtual bool good() = 0;
        virtual bool fail() = 0;
        virtual SharedArray<T> read(const size_t n, StreamTask<T, SharedArray<T>> * operation = nullptr) = 0;
        // ! IMPORTANT - inclusive means we read and consume the until character.
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        virtual SharedArray<T> read(SharedArray<T> until = SharedArray<T>(), StreamTask<T, SharedArray<T>> * operation = nullptr, bool inclusive = true) = 0;
        virtual ssize_t write(T * p_buf, size_t size) = 0;
};

template<typename T, typename RT>
class StreamProcessor {
    public:
        ESharedPtr<LoopCriteria<T>> criteria;
        ESharedPtr<Collector<T, RT>> collector; 
        StreamProcessor() = default;
        StreamProcessor(ESharedPtr<LoopCriteria<T>> criteria, ESharedPtr<Collector<T, RT>> collector): criteria(criteria), collector(collector) {};

        static RT streamCollect(EStreamI<T> * s, ESharedPtr<LoopCriteria<T>> criteria_shared, StreamTask<T, RT> * task, ESharedPtr<Collector<T, RT>> collector_shared) {
            // ! IMPORTANT - not thread safe
            LoopCriteria<T> * criteria = ESHAREDPTR_GET_PTR(criteria_shared);
            Collector<T, RT> * collector = ESHAREDPTR_GET_PTR(collector_shared);

            if (task != nullptr) {
                task->collector = collector;
                task->criteria = criteria;
            }
 
            T el = s->peek();
            while (true == criteria->good(el, true)) {
                s->get();
                if (task == nullptr) {
                    collector->accumulate(el);
                } else {
                    task->perform(el);
                }
                el = s->peek();
            }

            if (task != nullptr) {
                task->flush();
                // ! IMPORTANT - make sure to reset collector and criteria references in the task, so that you can store tasks as instance variables without keeping collectors live.
                //                  especially needed for direct the streamCollect function
                task->collector = nullptr;
                task->criteria = nullptr;
            }
            return collector->collect();
        };

        virtual RT streamCollect(EStreamI<T> * s, StreamTask<T, RT> * task) {
            return StreamProcessor<T, RT>::streamCollect(s, this->criteria, task, this->collector);
        }
};

template<typename T>
class EStream: public EStreamI<T> {
    /*
        Read and Write from file descriptor
    */
    private:
        struct pollfd poll_fd;
        std::ios_base::iostate flags;
        T ungot_el;
        bool ungot;
        SharedArray<T> backing_arr;
    protected:
        T * buf;
        size_t buf_size;
        size_t cursor;
        size_t els_in_buffer;
        StreamProcessor<T, SharedArray<T>> read_processor;
        virtual bool readPastBuffer() {
            return this->cursor >= this->els_in_buffer;
        }
        virtual void fillBuffer() {
            this->cursor = 0;
            ssize_t ret = ::read(this->fd, this->buf, this->buf_size * sizeof(T));
            // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
            if (ret <= 0 || (size_t)ret > this->buf_size) {
                this->els_in_buffer = 0;
                loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
                this->flags |= std::ios_base::badbit;
                throw std::runtime_error("Read error.");
            } else {
                this->els_in_buffer = ret;
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld els from transport layer.\n", ret);
            }
        }
    public:
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        std::string stream_log; 
#endif
        int fd;
        EStream() = default;
        EStream(T * p_buf, const size_t p_buf_size) {
            ungot = false;
            ungot_el = 0xFF;
            flags = std::ios_base::goodbit;
            buf = p_buf;
            buf_size = p_buf_size;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown and flags are updated if read past buffer. (see fillBuffer implementation)
            fd = -1;
            els_in_buffer = p_buf_size;
            read_processor = StreamProcessor<T, SharedArray<T>>(
                initReadCriteria<T>(), 
                initReadCollector<T, SharedArray<T>>()
            );
        }
        EStream(const int fd): EStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        EStream(const int p_fd, const size_t p_buf_size) {
            ungot = false;
            ungot_el = 0xFF;
            flags = std::ios_base::goodbit;
            if (p_fd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (p_buf_size < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            poll_fd.fd = p_fd;
            poll_fd.events = POLLIN;
            buf_size = p_buf_size;
            cursor = 0;
            fd = p_fd;
            els_in_buffer = 0;
            buf = newCArray<T>(buf_size);
            read_processor = StreamProcessor<T, SharedArray<T>>(
                initReadCriteria<T>(),
                initReadCollector<T, SharedArray<T>>()
            );
        }
        virtual ~EStream() {
            // deleteCArray<T>(this->buf, this->buf_size);
            // lmao
        }

        // standard istream methods
        T get() override {
            T byte = this->peek();
            if (this->cursor == 0 && true == this->ungot) {
                this->ungot = false;
            }
            this->cursor++;
        #if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
            this->stream_log += byte;
            if (this->stream_log.size() > MAX_STREAM_LOG_SIZE) {
                this->stream_log.clear();
            }
        #endif
            return byte;
        }
        T peek() override {
            if (true == this->good()) {
                if (true == this->readPastBuffer()) {
                    this->ungot_el = this->buf[this->cursor];
                    this->ungot = false;
                    this->fillBuffer();
                }
            } else if (this->cursor == 0 && true == this->ungot) {
                return this->ungot_el;
            }
            return this->buf[this->cursor];
        }
        void unget() override {
            if (this->cursor == 0) { // fill buffer just called...
                this->ungot = true;
            } else {
                this->cursor--; // is the normal case...
            }
        }
        bool eof() override {
            return this->flags & std::ios_base::eofbit;
        }
        bool good() override {
            if (this->flags == 0) {
                if (true == this->readPastBuffer()) {
                    if (this->fd < 0) {
                        this->flags |= std::ios_base::badbit;
                    } else {
                        if (1 != poll(&this->poll_fd, 1, 0)) {
                            this->flags |= std::ios_base::badbit;
                        }
                    }
                }
            }
            return this->flags == 0;
        }
        bool fail() override {
            return this->flags & std::ios_base::failbit;
        }
        SharedArray<T> read(const size_t n, StreamTask<T, SharedArray<T>> * operation = nullptr) override {
            if (n == 0) {
                throw std::runtime_error("It doesn't make sense to read zero els.");
            } else if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
                throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
            }
            if (operation == nullptr) {
                SharedArray<T> data;
                size_t els_read = 0;
                if (true == this->readPastBuffer()) {
                    this->fillBuffer();
                } else if (this->cursor == 0 && true == this->ungot) {
                    this->ungot = false;
                    data.append(this->ungot_el);
                    els_read++;
                }
                while (els_read < n) {
                    size_t els_left_to_read = n - els_read;
                    size_t els_left_in_buffer = this->els_in_buffer - this->cursor;
                    if (els_left_to_read > els_left_in_buffer) {
                        // copy data left in buffer and read more
                        data.append(this->buf + this->cursor, els_left_in_buffer);
                        els_read += els_left_in_buffer;

                        this->fillBuffer();
                    } else {
                        // else enough data in buffer
                        data.append(this->buf + this->cursor, els_left_to_read);
                        this->cursor += els_left_to_read;
                        els_read += els_left_to_read;
                    }
                } 
                return data;
            } else {
                bool included = false;
                bool inclusive = true;
                SharedArray<T> until;
                *(ESHAREDPTR_GET_PTR(this->read_processor.criteria)) = LoopCriteriaInfo(LOOP_CRITERIA_UNTIL_NUM_ELEMENTS, included, inclusive, n, until);
                return this->read_processor.streamCollect(dynamic_cast<EStreamI<T> *>(this), operation);
            }
        }
        // ! IMPORTANT - inclusive means we read and consume the until character.
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        SharedArray<T> read(SharedArray<T> until, StreamTask<T, SharedArray<T>> * operation = nullptr, bool inclusive = true) override {
            bool included = false;
            size_t until_size = 0;
            *(ESHAREDPTR_GET_PTR(this->read_processor.criteria)) = LoopCriteriaInfo(LOOP_CRITERIA_UNTIL_MATCH, included, inclusive, until_size, until);
            return this->read_processor.streamCollect(dynamic_cast<EStreamI<T> *>(this), operation);
        }
        ssize_t write(T * p_buf, size_t size) override {
            return ::write(this->fd, (void *)p_buf, size * sizeof(T));
        }
        template<typename RT>
        RT streamCollect(ESharedPtr<LoopCriteria<T>> criteria, StreamTask<T, RT> * task, ESharedPtr<Collector<T, RT>> collector) {
            return StreamProcessor<T, RT>::streamCollect(this, criteria, task, collector);
        }

        // ! IMPORTANT - purposely not explicitly swaping for all variables for the following reasons:
        //      1. It gets complicated (and annoying) for heirarchical types and classes with many variables.
        //      2. The consequences aren't terrible, you should profile your application's memory usage and pivot accordingly.

        //      All of the documentation I read (at least as of 11/2024) states it's necessary even if it's a terrible implementation given the other language functionality.
        //      Logic might otherwise lead you to conclude it MUST "destroy old and assigns" or "swap then destroy new" automatically - regardless. 
        //          If I am reading this correctly, I think this is meant for the reasoners - if for anyone at all.

        //      Generally if the program calls new via a constructor, then explicitly define the move constructor and assignment. 
        //          Either swap all variables explictly or use default functionality (which presumably does what I described above?). 
        //          This ensures it doesn't copy - avoiding dangling shlongs...
        EStream(EStream && x) = default;
        EStream& operator=(EStream && x) = default;

        bool operator!() {
            return this->good();
        }
        // bool operator bool() {
        //     return this->good();
        // }
};
};

// @ static

// assuming amd64 - what year are we in? LMAO
// static_assert(sizeof(ByteEStream) ==
//     sizeof(char *) +
//     sizeof(size_t) +
//     sizeof(size_t) +
//     sizeof(size_t) +
//     sizeof(int) +
//     4 // just because?
// );
// static_assert(sizeof(ByteEStream) == 32);

#endif