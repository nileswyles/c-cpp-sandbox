#ifndef SEQUENCE_ID_GENERATOR_H
#define SEQUENCE_ID_GENERATOR_H

#include <memory>

#include <math.h>
#include <pthread.h>
#include <string>
#include <stdlib.h>

#include "datastructures/array.h"
#include "web/server_config.h"
#include "paths.h"
#include "file/file.h"
#include "estream/estream.h"
#include "string_utils.h"

// make sure global logger level is initialized
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_KEY_GENERATOR
#define LOGGER_LEVEL_KEY_GENERATOR GLOBAL_LOGGER_LEVEL
#endif

#ifndef LOGGER_KEY_GENERATOR
#define LOGGER_KEY_GENERATOR 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_KEY_GENERATOR

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_KEY_GENERATOR
#include "logger.h"

using namespace WylesLibs::File;

namespace WylesLibs {

class UniqueKeyGeneratorStore {
    private:
        std::shared_ptr<FileManager> file_manager;
        std::string file_path;
    public:
        UniqueKeyGeneratorStore() = default;
        UniqueKeyGeneratorStore(std::shared_ptr<FileManager> file_manager, std::string file_path): file_manager(file_manager), file_path(file_path) {}
        ~UniqueKeyGeneratorStore() = default;
        void refresh(uint64_t& current) {
            // read value from existing file...
            int fd = open(file_path.c_str(), O_RDONLY);
            if (fd != -1) {
                EStream r(fd);
                for (uint8_t i = 0; i < 16; i++) {
                    current = current << 4;
                    uint8_t byte = r.get();
                    if (isDigit(byte)) {
                        current |= byte - 0x30;
                    } else if (isUpperHex(byte)) {
                        current |= (byte - 0x41) + 0xA;
                    } else {
                        std::string msg = "Non-hex character detected.";
                        loggerPrintf(LOGGER_DEBUG, "%s: [0x%x] at iteration %u\n", msg.c_str(), byte, i);
                        throw std::runtime_error(msg);
                    }
                }
                close(fd);
            }
        }
        void flush(SharedArray<uint8_t> data) {
            if (this->file_path.size() > 0) {
                loggerPrintf(LOGGER_DEBUG, "Flushing sequence to data store at %s\n", this->file_path.c_str());
                this->file_manager->write(this->file_path, data, false);
            }
        }
};

class UniqueKeyGenerator {
    private:
        UniqueKeyGeneratorStore store;
        pthread_mutex_t mutex;
        uint64_t current;
        bool default_constructed;
    public:
        static void valToHexCharArray(SharedArray<uint8_t> data, uint64_t value, uint8_t bytes) {
            if (bytes > 8) {
                throw std::runtime_error("byte count cannnot exceed 64 bits.");
            }
            for (uint8_t i = 0; i < bytes; i++) {
                uint8_t low = (value & 0x0F);
                if (low < 0xA) {
                    low += 0x30;
                } else {
                    low += 0x41 - 0xA;
                }
                data.insert(0, low);
                uint8_t high = ((value & 0xF0) >> 4);
                if (high < 0xA) {
                    high += 0x30;
                } else {
                    high += 0x41 - 0xA;
                }
                data.insert(0, high);
                value = value >> 8; 
            }
            loggerPrintf(LOGGER_DEBUG, "Value: %lu, Hex Char Array: %s\n", value, data.toString().c_str());
        }
        UniqueKeyGenerator() {
            default_constructed = true;
        }
        UniqueKeyGenerator(ServerConfig config, UniqueKeyGeneratorStore store): store(store) {
            current = 0;
            store.refresh(current);
            pthread_mutex_init(&mutex, nullptr);
            default_constructed = false;
        }
        virtual ~UniqueKeyGenerator() {
            // ! IMPORTANT - because base destructor is implictly called by derived classes. This appears to be the best solution at the moment.
            if (false == default_constructed) {
                pthread_mutex_lock(&this->mutex);
                SharedArray<uint8_t> data;
                UniqueKeyGenerator::valToHexCharArray(data, current, 8);
                store.flush(data);
                pthread_mutex_unlock(&this->mutex);
                pthread_mutex_destroy(&this->mutex);
            }
        }
        virtual std::string next() {
            pthread_mutex_lock(&this->mutex);
            this->current++;

            SharedArray<uint8_t> data;
            UniqueKeyGenerator::valToHexCharArray(data, current, 8);
            pthread_mutex_unlock(&this->mutex);
            return data.toString();
        }
        virtual uint64_t nextAsInteger() {
            pthread_mutex_lock(&this->mutex);
            this->current++;
            pthread_mutex_unlock(&this->mutex);
            return current;
        }
};

typedef void (* UUIDGeneratorInitializer)(uint8_t&, uint8_t);

class UUIDGeneratorV4: public UniqueKeyGenerator {
    public:
        uint8_t num_randoms;
        static void initialize(uint8_t& num_randoms, uint8_t val) {
            if (val < 4 || val > 6) {
                throw std::runtime_error("Yeah, have you considered counseling? LOL...");
            } else {
                num_randoms = val;
            }
        }
        UUIDGeneratorV4(): UUIDGeneratorV4(4, UUIDGeneratorV4::initialize) {}
        UUIDGeneratorV4(uint8_t num_randoms): UUIDGeneratorV4(num_randoms, UUIDGeneratorV4::initialize) {}
        UUIDGeneratorV4(uint8_t randoms_arg, UUIDGeneratorInitializer initializer) {
            initializer(num_randoms, randoms_arg);
        }
        ~UUIDGeneratorV4() override = default;
        std::string next() override {
            SharedArray<uint8_t> data;
            for (uint8_t i = 0; i < num_randoms; i++) {
                UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)random(), 4);
            }
            return data.toString();
        }
};

class UUIDGeneratorV7: public UUIDGeneratorV4 {
    public:
        static void initialize(uint8_t& num_randoms, uint8_t val) {
            if (val < 2 || val > 4) {
                throw std::runtime_error("Yeah, just use UUIDGeneratorV4 LOL...");
            } else {
                num_randoms = val;
            }
        }
        UUIDGeneratorV7(): UUIDGeneratorV7(2) {}
        UUIDGeneratorV7(uint8_t num_randoms): UUIDGeneratorV4(num_randoms, UUIDGeneratorV7::initialize) {}
        ~UUIDGeneratorV7() override = default;
        std::string next() final override {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);

            SharedArray<int> selections;
            SharedArray<uint8_t> data;
            uint8_t num_selections = 2 + this->num_randoms;
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "Num Selections: %u\n", num_selections);
            for (uint8_t i = 0; i < num_selections; i++) {
#ifdef KEY_GENERATOR_DEBUG
                if (i != 0) {
                    // insert dash before every iteration, except first...
                    data.insert(0, '-');
                }
#endif
                int normalized_select = -1;
                if (i >= num_selections - 2) {
                    // i.e. if num_selections == 4 && i == 2, 3; where i range is [0, 3]
                    if (false == selections.contains(0)) {
                        normalized_select = 0;
                    } else if (false == selections.contains(1)) {
                        normalized_select = 1;
                    }
                }
                if (normalized_select == -1) {
                    int select = rand();
                    normalized_select = (int)round(num_selections * (double)select/RAND_MAX);
                    while (true == selections.contains(normalized_select)) {
                        select = rand();
                        normalized_select = (int)round(num_selections * (double)select/RAND_MAX);
                    }
                }
                loggerPrintf(LOGGER_DEBUG_VERBOSE, "Selection: %d\n", normalized_select);
                selections.append(normalized_select);
                if (normalized_select == 0) {
                    uint64_t seconds = (uint64_t)ts.tv_sec;
                    uint64_t random_mask = 0;
                    bool whitespacing = true;
                    for (uint8_t i = 0; i < 64; i++) {
                        uint8_t bit = ((seconds << i) >> 63) & 0x01;
                        // shift in new bit indiscriminently 
                        random_mask = random_mask << 1;
                        if (true == whitespacing) {
                            if (bit != 0) {
                                // found non-zero bit, no longer whitespacing!
                                whitespacing = false;
                            } else {
                                random_mask |= 0x1;
                            }
                        }
                    }
                    // printf("random_mask: %lx\n", random_mask);
                    random_mask &= (uint64_t)random() << 32 | (uint32_t)random();
                    UniqueKeyGenerator::valToHexCharArray(data, seconds ^ random_mask, 8);
                } else if (normalized_select == 1) {
#ifdef KEY_GENERATOR_DEBUG
                    data.insert(0, 't');
#endif
                    UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)ts.tv_nsec, 4);
                } else if (normalized_select >= 2) {
#ifdef KEY_GENERATOR_DEBUG
                    data.insert(0, 'r');
#endif
                    UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)random(), 4);
                }
            }
            return data.toString();
        }
};
}
#endif 