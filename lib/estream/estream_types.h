#ifndef WYLESLIBS_ESTREAM_TYPES_H
#define WYLESLIBS_ESTREAM_TYPES_H

#include "datastructures/array.h"
#include "memory/pointers.h"
#include "string_format.h"

#include <stdexcept>
#include <memory>

#include <stdint.h>
#include <stdbool.h>

#define ARBITRARY_SPATIAL_UNTIL_LIMIT 1 << 16

namespace WylesLibs {

    // @ criteria

    typedef enum LoopCriteriaMode {
        LOOP_CRITERIA_UNTIL_MATCH,
        LOOP_CRITERIA_UNTIL_NUM_ELEMENTS
    } LoopCriteriaMode;

    typedef enum LoopCriteriaState {
        LOOP_CRITERIA_STATE_NOT_GOOD = 0x00, // if no bit is set then not good

        LOOP_CRITERIA_STATE_GOOD = 0x01,
        LOOP_CRITERIA_STATE_AT_LAST = 0x02,
        LOOP_CRITERIA_STATE_GOOD_AT_LAST = 0x03, // if last bit is set, then it's good... if last two bits are set, then it's good and last iteration
    } LoopCriteriaState;

    template<typename T>
    class LoopCriteriaInfo {
        public:
            LoopCriteriaState state;
            LoopCriteriaMode mode;
            bool inclusive;
            size_t until_size;
            SharedArray<T> until;
            LoopCriteriaInfo(LoopCriteriaMode mode, bool inclusive, size_t until_size, SharedArray<T> until): 
                mode(mode), inclusive(inclusive), until_size(until_size), until(until), state(LOOP_CRITERIA_STATE_GOOD) {
            }
            virtual ~LoopCriteriaInfo() = default;
    };

    template<typename T>
    class LoopCriteria {
        protected:
            // TODO: because no compiler error if variable with same name is defined in derived class?
            //       This doesn't work...
            //        if (LOOP_CRITERIA_UNTIL_MATCH == LoopCriteriaInfo<uint8_t>::loop_criteria_info.mode) {
            LoopCriteriaInfo<T> loop_criteria_info;
            virtual LoopCriteriaState untilMatchNext(T& el) {
                bool until_match = this->loop_criteria_info.until.contains(el);
                if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_NOT_GOOD && until_match == false) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_GOOD;
                } else if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_GOOD && true == this->loop_criteria_info.inclusive && until_match == true) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_GOOD_AT_LAST;
                } else if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_GOOD && false == this->loop_criteria_info.inclusive && until_match == true) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_NOT_GOOD;
                } else if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_GOOD_AT_LAST) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_NOT_GOOD;
                }
                return this->loop_criteria_info.state;
            }
            virtual LoopCriteriaState untilSizeNext() {
                if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_NOT_GOOD && this->loop_criteria_info.until_size > 0) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_GOOD;
                } else if (this->loop_criteria_info.state == LOOP_CRITERIA_STATE_GOOD && this->loop_criteria_info.until_size <= 0) {
                    this->loop_criteria_info.state = LOOP_CRITERIA_STATE_NOT_GOOD;
                }
                this->loop_criteria_info.until_size--;
                return this->loop_criteria_info.state;
            }
        public:
            LoopCriteria(LoopCriteriaInfo<T> loop_criteria_info): loop_criteria_info(loop_criteria_info) {}
            virtual ~LoopCriteria() = default;
        
            virtual LoopCriteriaState nextState(T& el) {
                if (LOOP_CRITERIA_UNTIL_MATCH == this->loop_criteria_info.mode) {
                    if (++this->loop_criteria_info.until_size > ARBITRARY_SPATIAL_UNTIL_LIMIT) {
                        std::string msg = WylesLibs::format("Spatial until limit of {u} reached.", ARBITRARY_SPATIAL_UNTIL_LIMIT);
                        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
                        throw std::runtime_error(msg);
                    }
                    return this->untilMatchNext(el);
                } else {
                    return this->untilSizeNext();
                }
            }
            virtual LoopCriteriaState state() {
                return this->loop_criteria_info.state;
            }
    };

    // @ collector

    template<typename T, typename RT>
    class Collector {
        private:
        public:
            virtual ~Collector() = default;
            virtual void initialize() = 0;
            virtual void accumulate(T& el) = 0;
            virtual void accumulate(SharedArray<T>& els) {
                throw std::runtime_error("Accumulate function of multiple elements isn't implemented!");
            } // optional...
            virtual RT collect() = 0;
    };

    template<typename T>
    class ArrayCollector: public Collector<T, SharedArray<T>> {
        private:
            SharedArray<T> data;
        public:
            ArrayCollector() = default;
            virtual ~ArrayCollector() = default;
            void initialize() override final {
                this->data = SharedArray<T>();
            }
            void accumulate(T& el) override final {
                this->data.append(el);
            }
            void accumulate(SharedArray<T>& els) override final {
                this->data.append(els);
            }
            SharedArray<T> collect() override final {
                return this->data;
            }
    };

    template<typename T, typename RT>
    class StreamTask {
        public:
            Collector<T, RT> * collector;
            LoopCriteria<T> * criteria;
            virtual ~StreamTask() = default;
            // Good example of CPP OOP
     
            // good example of "dynamic dispatch"?
            // Virtual allows calling function defined in original type during object creation. Functions without virtual will call the function defined in the "current type".
            //
            //  Common example is to define and use a base class to cleanly select different functionality...
            //      This means that you might cast a sub-class pointer to it's base class... Virtual is required to call the functionality in the sub-class after casting to base class.
            //  Calls to ReaderTaskChain->flush (if not virtual) would call this function regardless of how it's defined in sub-classes.
            //  By contrast, calls to perform (if virtual) call the function defined by the class-type at creation. *** Regardless of any casting/type-conversion along the way ***.
     
            // good example of pure functions
            //     There's {} (no-op) vs. =0, 
            //      a class with a pure function (=0) is abstract and cannot be instantiated, but can be used as a pointer type.
            //  This means the compiler throws an error if it doesn't find an implementation of the pure function in sub-classes. 
     
            // override and final (called virt-specifier -- in CPP grammar specification)
            // These appear to be optional and more a formality thing... Restrict behaviour as much as possible if not strictly necessary...
     
            //  override says that function is virtual (supports dynamic dispatch) and overrides a virtual class.
            //      - enables compiler check to ensure base class function is virtual...  
            //  final says that function is virtual (supports dynamic dispatch) and cannot be overridden.
            //      - compiler error is generated if user tries to override. 
     
            virtual void flush() = 0;
            virtual void perform(T& el) = 0;
            virtual LoopCriteriaState criteriaState() {
                if (this->criteria == nullptr) {
                    std::string msg("Failed to get loop criteria. The criteria object was not initialized for this StreamTask.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    return this->criteria->state();
                }
            }
            virtual void collectorAccumulate(T& el) {
                if (this->collector == nullptr) {
                    std::string msg("Failed to accumulate element. The collector object was not initialized for this StreamTask.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    this->collector->accumulate(el);
                }
            }
            virtual void collectorAccumulate(SharedArray<T>& els) {
                if (this->collector == nullptr) {
                    std::string msg("Failed to accumulate array. The collector object was not initialized for this StreamTask.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    this->collector->accumulate(els);
                }
            }
    };

};

#endif