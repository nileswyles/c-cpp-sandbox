#ifndef WYLESLIBS_ESTREAM_TYPES_H
#define WYLESLIBS_ESTREAM_TYPES_H

#include "datastructures/array.h"

#include <memory>

#include <stdint.h>
#include <stdbool.h>

namespace WylesLibs {
    // @ criteria
    typedef enum LoopCriteriaMode {
        LOOP_CRITERIA_UNTIL_MATCH,
        LOOP_CRITERIA_UNTIL_NUM_ELEMENTS
    } LoopCriteriaMode;

    template<typename T>
    class LoopCriteria {
        protected:
            bool included;
            virtual bool untilMatchGood(T& el) {
                bool until_match = this->until.contains(el);
                printf("el: %c, until: '%s', until_match: %d\n", el, this->until.toString().c_str(), until_match);
                if (true == this->inclusive) {
                    if (true == until_match && false == this->included) {
                        this->included = true;
                        return true;
                    } else if (true == this->included) {
                        return false; // we are done, regardless of whether match or not.
                    } 
                }
                return false == until_match;
            }
            bool untilSizeGood() {
                return this->until_size-- > 0;
            }
        public:
            bool inclusive;
            size_t until_size;
            SharedArray<T> until;
            LoopCriteriaMode mode;

            LoopCriteria(): mode(LOOP_CRITERIA_UNTIL_MATCH) {}
            ~LoopCriteria() = default;
        
            virtual bool good(T& el) {
                if (LOOP_CRITERIA_UNTIL_MATCH == mode) {
                    return this->untilMatchGood(el);
                } else {
                    return this->untilSizeGood();
                }
            }
    };

    // TODO: 
    //  eventually you might want to initialize these with some information... so, this should accept another shared_ptr with that information.
    template<typename T>
    std::shared_ptr<LoopCriteria<T>> initReadCriteria() {
        return std::make_shared<LoopCriteria<T>>();
    }

    // @ collector

    template<typename T, typename RT>
    class Collector {
        public:
            ~Collector() = default;
            virtual void accumulate(T& el) = 0;
            virtual void accumulate(SharedArray<T>& els) {
                throw std::runtime_error("Accumulate function of multiple elements isn't implemented!");
            } // optional...
            virtual RT collect() = 0;
    };

    // TODO: 
    //  eventually you might want to initialize these with some information... so, this should accept another shared_ptr with that information.
    template<typename T, typename RT>
    std::shared_ptr<Collector<T, RT>> initReadCollector() {
        std::string msg("A specialization of initReadCollector is required for this datatype.");
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }

    template<typename T, typename RT>
    class StreamTask {
        public:
            std::shared_ptr<Collector<T, RT>> collector;
            std::shared_ptr<LoopCriteria<T>> criteria;
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
            virtual void collectorAccumulate(T& el) {
                // TODO: regarding this... are unit tests enough to not need this? concern being if refactor and don't consider this? just a thought... might end up null checking most of the time.
                if (this->collector.get() == nullptr) {
                    std::string msg("Failed to accumulate element. The collector object was not initialized for this Reader Task.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    this->collector->accumulate(el);
                }
            }
            virtual void collectorAccumulate(SharedArray<T>& els) {
                if (this->collector.get() == nullptr) {
                    std::string msg("Failed to accumulate array. The collector object was not initialized for this Reader Task.");
                    loggerPrintf(LOGGER_DEBUG, "Exception: %s\n", msg.c_str());
                    throw std::runtime_error(msg);
                } else {
                    this->collector->accumulate(els);
                }
            }
    };

};

#endif