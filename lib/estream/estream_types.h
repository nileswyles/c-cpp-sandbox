#ifndef WYLESLIBS_ESTREAM_TYPES_H
#define WYLESLIBS_ESTREAM_TYPES_H

#include "datastructures/array.h"

#include <memory>
#include "memory/pointers.h"

#include <stdint.h>
#include <stdbool.h>

namespace WylesLibs {

    // @ criteria

    typedef enum LoopCriteriaMode {
        LOOP_CRITERIA_UNTIL_MATCH,
        LOOP_CRITERIA_UNTIL_NUM_ELEMENTS
    } LoopCriteriaMode;

    template<typename T>
    class LoopCriteriaInfo {
        public:
            LoopCriteriaMode mode;
            bool included;
            bool inclusive;
            size_t until_size;
            SharedArray<T> until;
            LoopCriteriaInfo(LoopCriteriaMode mode, bool included, bool inclusive, size_t until_size, SharedArray<T> until): 
                mode(mode), included(included), inclusive(inclusive), until_size(until_size), until(until) {
            }
            virtual ~LoopCriteriaInfo() = default;
    };

    template<typename T>
    class LoopCriteria {
        protected:
            bool is_good;
            virtual bool untilMatchGood(T& el, bool is_new_char) {
                bool until_match = this->loop_criteria_info.until.contains(el);
                if (true == is_new_char) {
                    // Determine whether you should proceed to process the provided character...
                    if (true == this->loop_criteria_info.inclusive) {
                        if (true == this->loop_criteria_info.included) {
                            until_match = true; // we are done, regardless of whether match or not.
                        } else if (true == until_match && false == this->loop_criteria_info.included) {
                            this->loop_criteria_info.included = true; // allow one more loop
                            until_match = false;
                        } 
                    }
                    this->is_good = false == until_match;
                } else {
                    // Determining whether the element being processed is the until match...
                    this->is_good = false == until_match;
                }
                return this->is_good;
            }
            virtual bool untilSizeGood(bool is_new_char) {
                if (true == is_new_char) {
                    this->is_good = this->loop_criteria_info.until_size-- > 0;
                }
                return this->is_good;
            }
        public:
            // TODO: because no compiler error if variable with same name is defined in derived class?
            //       This doesn't work...
            //        if (LOOP_CRITERIA_UNTIL_MATCH == LoopCriteriaInfo<uint8_t>::loop_criteria_info.mode) {
            LoopCriteriaInfo<T> loop_criteria_info;
            LoopCriteria(LoopCriteriaInfo<T> loop_criteria_info): loop_criteria_info(loop_criteria_info), is_good(false) {}
            virtual ~LoopCriteria() = default;
        
            virtual bool good(T& el, bool is_new_char = false) {
                if (LOOP_CRITERIA_UNTIL_MATCH == this->loop_criteria_info.mode) {
                    return this->untilMatchGood(el, is_new_char);
                } else {
                    return this->untilSizeGood(is_new_char);
                }
            }
    };

    // TODO: 
    //  eventually you might want to initialize these with some loop_criteria_information... so, this should accept another shared_ptr with that loop_criteria_information.
    template<typename T>
    ESharedPtr<LoopCriteria<T>> initReadCriteria() {
        return ESharedPtr<LoopCriteria<T>>(
            new LoopCriteria<T>(
                LoopCriteriaInfo<T>(LOOP_CRITERIA_UNTIL_MATCH, false, true, 0, SharedArray<T>())
            )
        );
    }

    // @ collector

    template<typename T, typename RT>
    class Collector {
        public:
            virtual ~Collector() = default;
            virtual void accumulate(T& el) = 0;
            virtual void accumulate(SharedArray<T>& els) {
                throw std::runtime_error("Accumulate function of multiple elements isn't implemented!");
            } // optional...
            virtual RT collect() = 0;
    };

    template<typename T, typename RT>
    ESharedPtr<Collector<T, RT>> initReadCollector() {
        std::string msg("A specialization of initReadCollector is required for this datatype.");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

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
            virtual bool criteriaGood(T& el) {
                if (this->criteria == nullptr) {
                    std::string msg("Failed to get loop criteria. The criteria object was not initialized for this StreamTask.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    return this->criteria->good(el);
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