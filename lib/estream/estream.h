#ifndef WYLESLIBS_ESTREAM_H
#define WYLESLIBS_ESTREAM_H

#include "estream/estream_types.h"
#include "memory/pointers.h"
#include "datastructures/array.h"
#include "string_format.h"
#include "string_utils.h"

#include <string>
#include <stdexcept>
#include <memory>
#include <ios>
#include <istream>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>

#if defined(_MSC_VER)
#include "msc_poll.h"
#include "msc_unistd.h"
#else
#include <poll.h>
#include <unistd.h>
#endif

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
        virtual SharedArray<T> readEls(const size_t n, StreamTask<T, SharedArray<T>> * operation = nullptr) = 0;
        // ! IMPORTANT - inclusive means we read and consume the until character.
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        virtual SharedArray<T> read(SharedArray<T> until = SharedArray<T>(), StreamTask<T, SharedArray<T>> * operation = nullptr, bool inclusive = true) = 0;
        virtual int64_t write(T * b, size_t size) = 0;
};

template<typename T>
class EStream: public EStreamI<T> {
    /*
        Read and Write from file descriptor
    */
    private:
        static void initStreamProcessing(LoopCriteria<T> *& until_size_criteria, ArrayCollector<T> *& array_collector) {
            until_size_criteria = new LoopCriteria<T>(
                LoopCriteriaInfo<T>(LOOP_CRITERIA_UNTIL_MATCH, true, 0, SharedArray<T>())
            );
            array_collector = new ArrayCollector<T>();
        }
        struct pollfd poll_fd;
        T ungot_el;
        bool ungot;
        bool new_buffer;
    protected:
        T * buffer;
        size_t buffer_size;
        size_t els_in_buffer;
        LoopCriteria<T> * until_size_criteria;
        ArrayCollector<T> * array_collector;
        std::ios_base::iostate flags;
        bool readPastBuffer() override {
            return this->cursor >= this->els_in_buffer;
        }
        void fillBuffer() override {
            this->cursor = 0;
            
            int64_t ret = ::read(this->fd, this->buffer, this->buffer_size * sizeof(T));
            // IMPORTANT - STRICTLY BLOCKING FILE DESCRIPTORS!
            if (ret <= 0 || (size_t)ret > this->buffer_size * sizeof(T)) {
                this->els_in_buffer = 0;
                loggerPrintf(LOGGER_INFO, "Read error: %d, ret: %ld\n", errno, ret);
                this->flags |= std::ios_base::badbit;
                throw std::runtime_error("Read error.");
            } else {
                this->els_in_buffer = ret / sizeof(T);
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Read %ld els from transport layer.\n", ret);
            }
        }
    public:
        size_t cursor;
#if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
        std::string stream_log; 
#endif
        int fd;
        // TODO: this is hella lame... lol think about default constructor stuff some more...
        //      could alternatively initialize with nullptr and size 0 args from istreamestream. but that doesn't seem valid?
        EStream() {
            ungot = false;
            ungot_el = T();
            flags = std::ios_base::goodbit;
            buffer_size = 0;
            buffer = nullptr;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown and flags are updated if read past buffer. (see fillBuffer implementation)
            fd = -1;
            new_buffer = false;
            els_in_buffer = 0;
            EStream::initStreamProcessing(until_size_criteria, array_collector);
        }
        EStream(T * b, const size_t bs) {
            ungot = false;
            ungot_el = T();
            flags = std::ios_base::goodbit;
            buffer_size = bs;
            buffer = b;
            cursor = 0;
            // ! IMPORTANT - an exception is thrown and flags are updated if read past buffer. (see fillBuffer implementation)
            fd = -1;
            new_buffer = false;
            els_in_buffer = bs;
            EStream::initStreamProcessing(until_size_criteria, array_collector);
        }
        EStream(const int fd): EStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
        EStream(const int p_fd, const size_t bs) {
            if (p_fd < 0) {
                throw std::runtime_error("Invalid file descriptor provided.");
            }
            if (bs < 1) {
                throw std::runtime_error("Invalid buffer size provided.");
            }
            ungot = false;
            ungot_el = T();
            flags = std::ios_base::goodbit;
            buffer_size = bs;
            buffer = newCArray<T>(buffer_size);
            cursor = 0;
            fd = p_fd;
            new_buffer = true;
            els_in_buffer = 0;
            EStream::initStreamProcessing(until_size_criteria, array_collector);
            poll_fd.fd = p_fd;
            poll_fd.events = POLLIN;
        }
        virtual ~EStream() {
            if (true == this->new_buffer) {
                deleteCArray<T>(this->buffer, this->buffer_size);
            }
            delete this->until_size_criteria;
            delete this->array_collector;
        }

        // standard istream methods
        T get() override {
            T el = this->peek();
            if (this->cursor == 0 && true == this->ungot) {
                this->ungot = false;
            } else {
                this->cursor++;
            }
        #if ESTREAM_STREAM_LOG_ENABLE == 1 && GLOBAL_LOGGER_LEVEL >= LOGGER_DEBUG
            this->stream_log += el;
            if (this->stream_log.size() > MAX_STREAM_LOG_SIZE) {
                this->stream_log.clear();
            }
        #endif
            return el;
        }
        T peek() override {
            if (true == this->good()) {
                if (true == this->readPastBuffer()) {
                    if (this->cursor != 0) {
                        this->ungot_el = this->buffer[this->cursor - 1];
                    }
                    this->fillBuffer();
                }
            } else {
                // throw exception, if using status bits then you should check good outside of this.
                std::string msg("Read error. No more data in the stream to fill buffer.");
                loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            if (this->cursor == 0 && true == this->ungot) {
                return this->ungot_el;
            }
            return this->buffer[this->cursor];
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
        // TODO: overloading must work
        SharedArray<T> readEls(const size_t n, StreamTask<T, SharedArray<T>> * operation = nullptr) override {
            if (n == 0) {
                throw std::runtime_error("It doesn't make sense to read zero els.");
            } else if (n > ARBITRARY_LIMIT_BECAUSE_DUMB) {
                throw std::runtime_error("You're reading more than the limit specified... Read less, or you know what, don't read at all.");
            }
            SharedArray<T> data;
            if (operation == nullptr) {
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
                        data.append(this->buffer + this->cursor, els_left_in_buffer);
                        els_read += els_left_in_buffer;

                        this->fillBuffer();
                    } else {
                        // else enough data in buffer
                        data.append(this->buffer + this->cursor, els_left_to_read);
                        this->cursor += els_left_to_read;
                        els_read += els_left_to_read;
                    }
                } 
            } else {
                bool inclusive = true;
                SharedArray<T> until;
                *this->until_size_criteria = LoopCriteria<T>(LoopCriteriaInfo<T>(LOOP_CRITERIA_UNTIL_NUM_ELEMENTS, inclusive, n, until));
                data = this->streamCollect<SharedArray<T>>(this->until_size_criteria, operation, dynamic_cast<Collector<T, SharedArray<T>> *>(this->array_collector));
            }
            return data;
        }
        // ! IMPORTANT - inclusive means we read and consume the until character.
        //      inclusive value of false means the until character stays in the read buffer for the next read.
        //      Otherwise, SharedArray provides a method to cleanly remove the until character after the fact.
        //      The default value for the inclusive field is TRUE.
        SharedArray<T> read(SharedArray<T> until, StreamTask<T, SharedArray<T>> * operation = nullptr, bool inclusive = true) override {
            size_t until_size = 0;
            *this->until_size_criteria = LoopCriteria<T>(LoopCriteriaInfo<T>(LOOP_CRITERIA_UNTIL_MATCH, inclusive, until_size, until));
            return this->streamCollect<SharedArray<T>>(this->until_size_criteria, operation, dynamic_cast<Collector<T, SharedArray<T>> *>(this->array_collector));
        }
        int64_t write(T * b, size_t size) override {
            return ::write(this->fd, (void *)b, size * sizeof(T));
        }
        template<typename RT>
        RT streamCollect(LoopCriteria<T> * criteria, StreamTask<T, RT> * task, Collector<T, RT> * collector) {
            if (criteria == nullptr) {
                std::string msg("Criteria argument is nullptr.");
                loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            if (collector == nullptr) {
                std::string msg("Collector argument is nullptr.");
                loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
                throw std::runtime_error(msg);
            }

            collector->initialize();

            // ! IMPORTANT - not thread safe
            if (task != nullptr) {
                task->initialize();

                task->collector = collector;
                task->criteria = criteria;
            }
 
            T el = this->peek();
            // TODO:
            //  would be interesting to support multiple criteria and specify some logic about how they relate... maybe a criteria that combines some?
            //  the thought here is maybe I can create a better abstraction for state machines or logic?
            //  dry state-machines, lol idk... 
            //  criteria1 or criteria2 and criteria3 where criteria are themselves state machines?
            //  maybe not very useful.
            while (criteria->nextState(el) & LOOP_CRITERIA_STATE_GOOD) {
                this->get();
                if (task == nullptr) {
                    collector->accumulate(el);
                } else {
                    task->perform(el);
                }
                if (false == this->good() && criteria->state() & LOOP_CRITERIA_STATE_AT_LAST) {
                    // if no more data in the buffer but already at until then just break. else try to read regardless of whether good and throw exception if no more data.
                    break;
                }
                el = this->peek();
            }

            if (task != nullptr) {
                task->flush();
                // ! IMPORTANT - make sure to reset collector and criteria references in the task, so that you can store tasks as instance variables without keeping collectors live.
                //                  especially needed for direct the streamCollect function
                task->collector = nullptr;
                task->criteria = nullptr;
            }
            return collector->collect();
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
