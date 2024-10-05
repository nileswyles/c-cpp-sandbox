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
            printf("Refreshed store...\n");
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

// # BIG O! UniqueKeyGenerator for optimal performance!
// In this case, for file resources the file-based sequential stuff pairs nicely.
//  for almost everything else probably not LMAO
//  
class UUIDGeneratorV4: public UniqueKeyGenerator {
    protected:
   public:
        UUIDGeneratorV4() {}
        virtual ~UUIDGeneratorV4() {}
        std::string next() override {
            // random, independent variables, (1/2^128 * 1/2^128) 
            //   better?

            // counter intiuitive, revisit 
            Array<uint8_t> data;
            for (size_t i = 0; i < 4; i++) {
                UniqueKeyGenerator::valToHexCharArray(data, (uint32_t)random(), 4);
            }
            return data.toString();
        }
};

class UUIDGeneratorV7: public UUIDGeneratorV4 {
    private:
        uint8_t num_randoms;
    public:
        UUIDGeneratorV7(): num_randoms(2) {}
        UUIDGeneratorV7(uint8_t pNum_randoms) {
            if (pNum_randoms < 2) {
                throw std::runtime_error("Yeah, just use UUIDGeneratorV4 LOL...");
            } else {
                num_randoms = pNum_randoms;
            }
        }
        std::string next() final override {
            // Think about common SIG... 

            // okay, so we can take timestamp, machine specific and random value...

            // more info in general == less chance of collision.
            //  more random values == less chance of collision.

            // but naturally programmers should (normally) run from (or towards?) undeterminism... 

            //  If a deterministic solution isn't possible... (can't predict output from input?)

            //  Then some probabilistic model is all (a useful? sometimes) there is to providing confidence... 
            //      in making a decision (especially when designing a system).
            //      Similarly, more data == more accurate probabilistic model LMAO. 

            //  If that doesn't work, then throw out what you're trying to do? LMAO Sure if you don't really want it, or are a pussy... (like me LMAO?)

            //  As part of designing that probabilistic model, you need to determine (whether to consider weight? (i.e. all equally weighted, like here?))
            //      weight of each feature in the data-set... hmm... maybe just always individually assess weights...

            //  benefit of the doubt...
            //  
            //  alright, so regarding the UUID generator, what characterizes that probabilitic model? (english not englishing?, no I think that might be right? lol, who cares)
            // 
            //  my bad attempt of generating crude model
            //      when these probabilities are combined, it becomes a completely different thing?
            //  probability of generating UUID within interval of timestamps
            //      +
            //  Using the info above, you can pretty accurately predict and ensure uniqueness within a single generator function...

            //  probability of collision of random function..
            //      +
            //  Using the info above, you can pretty accurately predict and ensure uniquniess within a single machine...

            //  Now, I think the real interesting work starts here?
            //  probability of generating UUID within interval of timestamps become a function of the number of different machines (which is technically infinite)
            //  machine info fixes that?

            //  probabiltity of collision of unique machine specific info...
            //      +

            // alright... yeah, so basically said nothing here lol... yes, more (good?) data == more accurate... 
            // yeah, that means a simple model is sometimes better and just fine. 
            //  and so, maybe I need to get better at that? or do I need to adjust the other direction... ehh... 1d search space is lame (what a fucking paradox)...
            //  some data (outliers? what are outliers (easy in 2d model), ehhh tangent, clustering is interesting...) might through model completely out of wack

            // back from new tangent,

            //  hmm... yeah maybe there isn't much too it?
            //  probability of generating UUID within interval in machine... is fairly straightforward within a single machine? 
            //  probabiltiy of unique machine info is not as straight forward but can be estimated to I guess a comfortable place...

            // faith in humanity is important.
            //
            //  then there's reality... argument for decentralization (again pro's and cons)
            //      (house of representatives vs senate, again founding fathers were genius sheesh...) locality, 
            //      
            //  probability of random function collision very important, let's look into that...
            //  

            //      alright, yeah, so maybe calculating these probabilities are futile... 
            //          is one school of thought

            //      the other is restricting access to info? and trying to calculate probabilities?

            //  , a keen application developer should retry, gracefully handle that event? -- keyword keen... lmao
            //      hmmm...
            //
            //      something tells me both are valid, and since we know both to be true.
            //          locality...
            //      
            //      so, yes to sequence id generator but for a different reason! (More reason for sequence id generator).
            //
            //      that said, there's tremendous value in an unbiased opinion?  

            //          how could anyone commit to anything LMAO...
            //          bruh big i.

            //      ahh, you have both... local and external must work together somehow?

            //      or faith in local?

            //      yeah, government/people are interesting... people are everything actually lol... A good balance of people is everything...
            //          My tolerance has windled lol

            //      TODO: read more writings by founding fathers...
            //              read about sources of entropy and attempts at probabilitic modeling (quantum mechanics?)

            // All that encapsulated by invoking/implementing uuid? everything is deep?
            

            // Alright, implementing should be straight forward?

            // how much random?

            // TIMESTAMP + MAC_ADDRESS | UNIQUE_STRING | + RANDOM SEQUENCES

            // combined through what?
            //  AND
            //  XOR'd
            //  OR

            // at cat...? duh?

            // there might not be trade offs here?
            //  also, string vs integer (byte) representation doesn't matter as long as consistent?

            //  regardless, this is more computation than current_value++; lol... also, riskayyy

            // this isn't the most accurate model, but good enough to think about the concept.
            //  really need to bring in combinatorics (discrete maths)... 
            // 1/(cpufrequency(ghz)/1ghz) * (1/2^32 * 1/2^32) (* 1/3)
            //    less better? (more better?) LMAO
            // never-the-less, this certainly makes it more interesting? LMAO
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
            // In conclusion, be a keen developer and gracefully handle duplicates regardless of which UUID method is used?
            return data.toString();
        }
};
}
#endif 