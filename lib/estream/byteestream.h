#ifndef WYLESLIBS_BYTE_STREAM_H
#define WYLESLIBS_BYTE_STREAM_H

#include "estream/estream_types.h"
#include "estream/estream.h"
#include "estream/reader_task.h"
#include "datastructures/array.h"

#include <memory>
#include <tuple>

#include <stdint.h>

namespace WylesLibs {

    // @ criteria

    // ! IMPORTANT - decided to group the functionality like this to limit verbosity and minimize developer churn.
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

    class ByteCollector: public Collector<uint8_t, SharedArray<uint8_t>> {
        private:
            SharedArray<uint8_t> data;
        public:
            ByteCollector() = default;
            ~ByteCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            void accumulate(SharedArray<uint8_t>& cs) override final;
            SharedArray<uint8_t> collect() override final;
    };

    template<>
    ESharedPtr<Collector<uint8_t, SharedArray<uint8_t>>> initReadCollector<uint8_t, SharedArray<uint8_t>>();

    class NaturalCollector: public Collector<uint8_t, std::tuple<uint64_t, size_t>> {
        private:
            size_t digit_count;
            uint64_t value;
        public:
            NaturalCollector(): digit_count(0), value(0) {}
            ~NaturalCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            std::tuple<uint64_t, size_t> collect() override final;
    };

    class DecimalCollector: public Collector<uint8_t, std::tuple<double, size_t>> {
        private:
            double decimal_divisor;
            size_t digit_count;
            double value;
        public:
            DecimalCollector(): digit_count(0), value(0.0), decimal_divisor(10.0) {}
            ~DecimalCollector() override = default;
            void initialize() override final;
            void accumulate(uint8_t& c) override final;
            std::tuple<double, size_t> collect() override final;
    };

    class ByteEStream: public EStream<uint8_t> {
        private:
            static void initProcessors(StreamProcessor<uint8_t, std::tuple<uint64_t, size_t>>& natural_processor, 
                                        StreamProcessor<uint8_t, std::tuple<double, size_t>>& decimal_processor) {
                ESharedPtr<LoopCriteria<uint8_t>> char_class_criteria(
                    dynamic_cast<LoopCriteria<uint8_t>*>(
                        new ByteIsCharClassCriteria(ByteIsCharClassCriteria::DIGIT_CLASS)
                    )
                ); 
                natural_processor = StreamProcessor<uint8_t, std::tuple<uint64_t, size_t>>(
                    char_class_criteria,
                    ESharedPtr<Collector<uint8_t, std::tuple<uint64_t, size_t>>>(
                        dynamic_cast<Collector<uint8_t, std::tuple<uint64_t, size_t>>*>(
                            new NaturalCollector
                        )
                    )
                );
                decimal_processor = StreamProcessor<uint8_t, std::tuple<double, size_t>>(
                    char_class_criteria,
                    ESharedPtr<Collector<uint8_t, std::tuple<double, size_t>>>(
                        dynamic_cast<Collector<uint8_t, std::tuple<double, size_t>>*>(
                            new DecimalCollector
                        )
                    )
                );
            }
        public:
            // this might be a bad example of this, because other implementation is arguably simpler but you'll see? lol
            StreamProcessor<uint8_t, std::tuple<uint64_t, size_t>> natural_processor;
            StreamProcessor<uint8_t, std::tuple<double, size_t>> decimal_processor;

            ByteEStream() {
                ByteEStream::initProcessors(natural_processor, decimal_processor);
            }
            ByteEStream(uint8_t * b, const size_t bs): EStream<uint8_t>(b, bs) {
                ByteEStream::initProcessors(natural_processor, decimal_processor);
            }
            ByteEStream(const int fd): ByteEStream(fd, READER_RECOMMENDED_BUF_SIZE) {}
            ByteEStream(const int p_fd, const size_t bs): EStream<uint8_t>(p_fd, bs) {
                ByteEStream::initProcessors(natural_processor, decimal_processor);
            }
            ~ByteEStream() override = default;

            virtual SharedArray<uint8_t> read(std::string until = "\n", ReaderTask * operation = nullptr, bool inclusive = true);
            virtual std::tuple<uint64_t, size_t> readNatural();
            virtual std::tuple<double, size_t, size_t> readDecimal();

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