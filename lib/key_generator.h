#ifndef SEQUENCE_ID_GENERATOR_H
#define SEQUENCE_ID_GENERATOR_H

#include <math.h>
#include <pthread.h>
#include <string>
#include <stdlib.h>

#include "array.h"
#include "web/server_config.h"
#include "paths.h"
#include "file.h"
#include "iostream/iostream.h"
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

namespace WylesLibs {

class UniqueKeyGeneratorStore {
    std::string file_path;
    public:
        UniqueKeyGeneratorStore() {}
        UniqueKeyGeneratorStore(std::string file_path): file_path(file_path) {}
        ~UniqueKeyGeneratorStore() = default;
        void refresh(uint64_t& current) {
            // read value from existing file...
            int fd = open(file_path.c_str(), O_RDONLY);
            if (fd != -1) {
                IOStream r(fd);
                for (size_t i = 0; i < 16; i++) {
                    current = current << 4;
                    uint8_t byte = r.readByte();
                    if (isDigit(byte)) {
                        current |= byte - 0x30;
                    } else if (isUpperHex(byte)) {
                        current |= (byte - 0x41) + 0xA;
                    } else {
                        std::string msg = "Non-hex character detected.";
                        loggerPrintf(LOGGER_DEBUG, "%s: [0x%x] at iteration %lu\n", msg.c_str(), byte, i);
                        throw std::runtime_error(msg);
                    }
                }
                close(fd);
            }
        }
        void flush(Array<uint8_t> data) {
            if (this->file_path.size() > 0) {
                loggerPrintf(LOGGER_DEBUG, "Flushing sequence to data store at %s\n", this->file_path.c_str());
                File::write(this->file_path, data, false);
            }
        }
};

class UniqueKeyGenerator {
    private:
        UniqueKeyGeneratorStore store;
        pthread_mutex_t mutex;
        uint64_t current;
    public:
        static void valToHexCharArray(Array<uint8_t> data, uint64_t value, size_t bytes) {
            if (bytes > 8) {
                throw std::runtime_error("byte count cannnot exceed 64 bits.");
            }
            for (size_t i = 0; i < bytes; i++) {
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
        UniqueKeyGenerator() {}
        // There is no correct solution... 
        UniqueKeyGenerator(ServerConfig config, UniqueKeyGeneratorStore store): store(store) {
            current = 0;
            store.refresh(current);
            pthread_mutex_init(&mutex, nullptr);
        }
        virtual ~UniqueKeyGenerator() {
            pthread_mutex_lock(&this->mutex);
            Array<uint8_t> data;
            UniqueKeyGenerator::valToHexCharArray(data, current, 8);
            store.flush(data);
            pthread_mutex_unlock(&this->mutex);
            pthread_mutex_destroy(&this->mutex);
        }
        virtual std::string next() {
            pthread_mutex_lock(&this->mutex);
            this->current++;

            Array<uint8_t> data;
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

class UUIDGeneratorV4: public UniqueKeyGenerator {
    public:
        uint8_t num_randoms;
        UUIDGeneratorV4(): UUIDGeneratorV4(4) {}
        UUIDGeneratorV4(uint8_t num_randoms): num_randoms(num_randoms) {
            if (num_randoms < 4 || num_randoms > 6) {
                throw std::runtime_error("Yeah, have you considered counseling? LOL...");
            } else {
                num_randoms = num_randoms;
            }
        }
        ~UUIDGeneratorV4() override = default;
        std::string next() override {
            Array<uint8_t> data;
            for (size_t i = 0; i < num_randoms; i++) {
                UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)random(), 4);
            }
            return data.toString();
        }
};

class UUIDGeneratorV7: public UUIDGeneratorV4 {
    public:
        UUIDGeneratorV7(): UUIDGeneratorV7(2) {}
        UUIDGeneratorV7(uint8_t num_randoms) {
            if (num_randoms < 2 || num_randoms > 4) {
                throw std::runtime_error("Yeah, just use UUIDGeneratorV4 LOL...");
            } else {
                num_randoms = num_randoms;
            }
        }
        ~UUIDGeneratorV7() override = default;
        std::string next() final override {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);

            Array<int> selections;
            Array<uint8_t> data;
            size_t size = 1 + this->num_randoms;
            for (size_t i = 0; i <= size; i++) {
                int select = rand();

                int normalized_select = (int)round(size * (double)select/RAND_MAX);
                while (selections.contains(normalized_select)) {
                    select = rand();
                    normalized_select = (int)round(size * (double)select/RAND_MAX);
                }
                selections.append(normalized_select);

                if (normalized_select == 0) {
                    UniqueKeyGenerator::valToHexCharArray(data, (uint64_t)ts.tv_sec, 8);
                } else if (normalized_select == 1) {
                    UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)ts.tv_nsec, 4);
                } else if (normalized_select >= 2) {
                    UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)random(), 4);
                }
            }
            return data.toString();
        }
};
}
#endif 