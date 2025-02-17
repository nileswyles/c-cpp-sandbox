#ifndef WYLESLIBS_BYTE_STREAM_H
#define WYLESLIBS_BYTE_STREAM_H

#include "estream/estream_types.h"
#include "estream/estream.h"
#include "estream/reader_task.h"
#include "datastructures/array.h"

#include <stdexcept>
#include <memory>
#include <tuple>

#include <stdint.h>

// as opposed to typedef automatically which would need to coerse?
#define NaturalTuple std::tuple<uint64_t, size_t>
#define DecimalTuple std::tuple<double, size_t, size_t>

namespace WylesLibs {

    // @ criteria

    // ! IMPORTANT - decided to group the functionality like this to limit verbosity and minimize developer churn.

    //  TODO: upgrade this to "primitive" lol? so, there's loop until character, loop number of elements, 
    //          then this is --- loop until negative match? or loop while class?
    //          hmm.... idk
    class ByteIsCharClassCriteria: public LoopCriteria<uint8_t> {
        private:
            LoopCriteriaState untilMatchNext(uint8_t& c) override final;
        public:
            static constexpr uint8_t NO_CLASS = 0x0;
            static constexpr uint8_t UPPER_HEX_CLASS = 0x1;
            static constexpr uint8_t LOWER_HEX_CLASS = 0x2;
            static constexpr uint8_t HEX_CLASS = 0x3; // UPPER_HEX_CLASS | LOWER_HEX_CLASS
            static constexpr uint8_t ALPHANUMERIC_CLASS = 0x4;
            static constexpr uint8_t DIGIT_CLASS = 0x8;
            uint8_t char_class;
            ByteIsCharClassCriteria(uint8_t char_class): char_class(char_class), LoopCriteria<uint8_t>(LoopCriteriaInfo<uint8_t>(LOOP_CRITERIA_UNTIL_MATCH, true, 0, SharedArray<uint8_t>())) {}
            ~ByteIsCharClassCriteria() override = default;
        
            // TODO: 
            // hmm.. extending LoopCriteriaState might be a pain? think about that some more? adding more statess? is that even a concern though?
            LoopCriteriaState nextState(uint8_t& c) override final;
    };

    // @ collectors

    class ByteStringCollector: public Collector<uint8_t, std::string> {
        private:
            std::string data;
        public:
            ByteStringCollector() = default;
            ~ByteStringCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            void accumulate(SharedArray<uint8_t>& cs) override final;
            std::string collect() override final;
    };

    class NaturalCollector: public Collector<uint8_t, NaturalTuple> {
        private:
            size_t digit_count;
            uint64_t value;
        public:
            NaturalCollector(): digit_count(0), value(0) {}
            ~NaturalCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            NaturalTuple collect() override final;
    };

    class DecimalCollector: public Collector<uint8_t, DecimalTuple> {
        private:
            double decimal_divisor;
            size_t digit_count;
            double value;
        public:
            DecimalCollector(): digit_count(0), value(0.0), decimal_divisor(10.0) {}
            ~DecimalCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            DecimalTuple collect() override final;
    };

    class ByteEStream: public EStream<uint8_t> {
        private:
            static void initByteStreamProcessing(Collector<uint8_t, NaturalTuple>** natural_collector,
                                        Collector<uint8_t, DecimalTuple>** decimal_collector,
                                        Collector<uint8_t, std::string>** string_collector) {
                *natural_collector = dynamic_cast<Collector<uint8_t, NaturalTuple> *>(
                    new NaturalCollector
                );
                *decimal_collector = dynamic_cast<Collector<uint8_t, DecimalTuple> *>(
                    new DecimalCollector
                );
                *string_collector = dynamic_cast<Collector<uint8_t, std::string> *>(
                    new ByteStringCollector
                );
            }
        public:
            Collector<uint8_t, NaturalTuple> * natural_collector;
            Collector<uint8_t, DecimalTuple> * decimal_collector;
            Collector<uint8_t, std::string> * string_collector;

            ByteEStream() {
                ByteEStream::initByteStreamProcessing(&natural_collector, &decimal_collector, &string_collector);
            }
            ByteEStream(uint8_t * b, const size_t bs): EStream<uint8_t>(b, bs) {
                ByteEStream::initByteStreamProcessing(&natural_collector, &decimal_collector, &string_collector);
            }
            ByteEStream(const int fd): ByteEStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
            ByteEStream(const int p_fd, const size_t bs): EStream<uint8_t>(p_fd, bs) {
                ByteEStream::initByteStreamProcessing(&natural_collector, &decimal_collector, &string_collector);
            }
            ~ByteEStream() {
                delete natural_collector;
                delete decimal_collector;
                delete string_collector;

                // ~EStream
            }

            template<typename RT>
            RT read(const size_t n, StreamTask<uint8_t, RT> * operation = nullptr) {
                return EStream<uint8_t>::read<RT>(n, operation);
            }
            template<typename RT>
            RT read(SharedArray<uint8_t> until, StreamTask<uint8_t, RT> * operation = nullptr, bool inclusive = true) {
                return EStream<uint8_t>::read<RT>(until, operation, inclusive);
            }
            // "" == DIGIT CLASS criteria
            // ( ͡° ͜ʖ ͡°) U+1F608 U+1FAF5
            // could do without this curly instead of quotes?
            template<typename RT>
            RT read(std::string until = "\n", StreamTask<uint8_t, RT> * operation = nullptr, bool inclusive = true) {
                return EStream<uint8_t>::read<RT>(SharedArray<uint8_t>(until), operation, inclusive);
            }

            ByteEStream(ByteEStream && x) = default;
            ByteEStream& operator=(ByteEStream && x) = default;
    };

    #ifdef WYLESLIBS_SSL_ENABLED
    class SSLEStream: public ByteEStream {
        /*
            Read and Write from openssl object
        */
        private:
            SSL * ssl;
            static SSL * acceptTLS(SSL_CTX * context, int fd, bool client_auth_enabled);
        protected:
            void fillBuffer() override final;
        public:
            SSLEStream() = default;
            SSLEStream(SSL_CTX * context, int fd, bool client_auth_enabled): ByteEStream(fd, READER_RECOMMENDED_BUF_SIZE_SSL) {
                ssl = acceptTLS(context, fd, client_auth_enabled);
            }
            ~SSLEStream() override final {
                if (this->ssl != nullptr) {
                    SSL_shutdown(this->ssl);
                    SSL_free(this->ssl);
                }
            }
            template<typename RT>
            RT read(const size_t n, StreamTask<uint8_t, RT> * operation = nullptr) {
                return EStream<uint8_t>::read<RT>(n, operation);
            }
            template<typename RT>
            RT read(SharedArray<uint8_t> until, StreamTask<uint8_t, RT> * operation = nullptr , bool inclusive = true) {
                return EStream<uint8_t>::read<RT>(until, operation, inclusive);
            }
            template<typename RT>
            RT read(std::string until = "\n", StreamTask<uint8_t, RT> * operation = nullptr, bool inclusive = true) {
                return EStream<uint8_t>::read<RT>(SharedArray<uint8_t>(until), operation, inclusive);
            }

            ssize_t write(uint8_t * b, size_t size) override final;

            SSLEStream(SSLEStream && x) = default;
            SSLEStream& operator=(SSLEStream && x) = default;

            bool operator!() {
                return this->good();
            }
            // bool operator bool() {
            //     return this->good();
            // }
    };
    #endif
};

#endif