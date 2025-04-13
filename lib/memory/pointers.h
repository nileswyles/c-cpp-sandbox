#ifndef WYLESLIBS_MEMORY_POINTERS_H
#define WYLESLIBS_MEMORY_POINTERS_H

#include "logger.h"
#include "string_format.h"

#include <stdexcept>
#include <memory>

#define ESHAREDPTR_GET_PTR(eshared_ptr) eshared_ptr.getPtr(__FILE__, __LINE__)
#define ESHAREDPTR_GET_REF(eshared_ptr) eshared_ptr.getRef(__FILE__, __LINE__)

namespace WylesLibs {
/*
    ! IMPORTANT - how does one think to arrive at this shared-weak design? In this situation, it's not obvious that the compiler infers left-operand type as template type when a generic type is used as a return type. 
                    That's certainly not in anything I read but is how the stdlib shared-weak works so it implied functionality - I guess? 
                    Let's expand on this process... is it just one thing leads to another? So, just build, iterate, pivot judiciously and converge (or don't).

                    As an aside, you can, more generally, use function templates to get around the circular dependency thing lmao... see estream.h should I update that? Probably not? in that situation a common interface was the better approach?
                        but yeah, a useful tool indeed.
*/

class ESharedPtrException: public std::runtime_error {
    public:
        /** Takes a character string describing the error.  */
        ESharedPtrException(const std::string& arg): std::runtime_error(arg) {}
        ESharedPtrException(const char * arg): std::runtime_error(arg) {}
 
        ESharedPtrException(ESharedPtrException&&) noexcept;
        ESharedPtrException& operator=(ESharedPtrException&&) noexcept;
};

template<typename T>
class EPointerControl {
    public:
        T * ptr;
        size_t * e_instance_count;

        EPointerControl(): ptr(nullptr), e_instance_count(new size_t(1)) {}
        EPointerControl(T * ptr): ptr(ptr), e_instance_count(new size_t(1)) {}
        EPointerControl(T * ptr, size_t * e_instance_count): ptr(ptr), e_instance_count(e_instance_count) {}
        ~EPointerControl() {
            delete this->ptr;
        }
};
template<typename T>
class ESharedPtr {
    public:
        EPointerControl<T> ** ctrl_container;
        /*
            ! IMPORTANT - don't provide a path where ctrl_container and *ctrl_container will ever be nullptr.
                             This allows for minimal checking.
        */
        ESharedPtr() {
            ctrl_container = new EPointerControl<T>*(new EPointerControl<T>);
        }
        ESharedPtr(EPointerControl<T> ** cc) {
            if (cc == nullptr || *cc == nullptr) {
                std::string msg("Cannot create an ESharedPtr from a nullptr ctrl class.");
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw ESharedPtrException(msg);
            }
            ctrl_container = cc;
            (*(*ctrl_container)->e_instance_count)++;
        }
        ESharedPtr(T * ptr) {
            ctrl_container = new EPointerControl<T>*(new EPointerControl<T>(ptr));
        }
        ~ESharedPtr() {
            if (this->ctrl_container != nullptr && *this->ctrl_container != nullptr) {
                ((*(*this->ctrl_container)->e_instance_count))--;
                if ((*(*this->ctrl_container)->e_instance_count) == 0) {
                    delete *this->ctrl_container;
                    delete this->ctrl_container;
                }
            }
        }
        T * getPtr(const char * file_name, int line) {
            if (true == this->isNullPtr()) {
                std::string msg = WylesLibs::format("Attempted to retrieve ptr at: {s}:{d}", file_name, line);
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw ESharedPtrException(msg);
            }
            return (*this->ctrl_container)->ptr;
        }
        T& getRef(const char * file_name, int line) {
            if (true == this->isNullPtr()) {
                std::string msg = WylesLibs::format("Attempted to retrieve ptr at: {s}:{d}", file_name, line);
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw ESharedPtrException(msg);
            }
            return *(*this->ctrl_container)->ptr;
        }
        bool isNullPtr() {
            return (*this->ctrl_container)->ptr == nullptr;
        }
        explicit operator bool() {
            return false == this->isNullPtr();
        }
        bool operator!() {
            return this->isNullPtr();
        }
        bool operator==(const ESharedPtr<T>& x) {
            return (*this->ctrl_container)->ptr == (*x.ctrl_container)->ptr;
        }
        bool operator!=(const ESharedPtr<T>& x) {
            return (*this->ctrl_container)->ptr != (*x.ctrl_container)->ptr;
        }
        ESharedPtr<T>& operator=(T * ptr) {
            (*this->ctrl_container)->ptr = ptr;
            return *this;
        }
        // Copy
        ESharedPtr(const ESharedPtr<T>& x) {
            this->ctrl_container = x.ctrl_container;
            (*(*this->ctrl_container)->e_instance_count)++;
        }
        ESharedPtr<T>& operator= (const ESharedPtr<T>& x) {
            this->ctrl_container = x.ctrl_container;
            (*(*this->ctrl_container)->e_instance_count)++;
            return *this;
        }
        // Move
        ESharedPtr(ESharedPtr<T>&& x) {
            std::swap(this->ctrl_container, x.ctrl_container);
        }
        ESharedPtr<T>& operator= (ESharedPtr<T>&& x) {
            std::swap(this->ctrl_container, x.ctrl_container);
            return *this;
        }

        // implicit casting
        template<typename C>
        ESharedPtr(ESharedPtr<C>& x) {
            ctrl_container = new EPointerControl<T>*(
                new EPointerControl<T>(
                    (T *)(*x.ctrl_container)->ptr,
                    (*x.ctrl_container)->e_instance_count
                )
            );
            (*(*ctrl_container)->e_instance_count)++;
        }
        template<typename C>
        ESharedPtr<T>& operator= (ESharedPtr<C>& x) {
            (*this->ctrl_container)->e_instance_count = (*x.ctrl_container)->e_instance_count;
            (*this->ctrl_container)->ptr = (T *)(*x.ctrl_container)->ptr;
            (*(*this->ctrl_container)->e_instance_count)++;
            return *this;
        }
        // explicit casting
        template<typename C>
        ESharedPtr<C> cast() {
            printf("%p\n", (C *)(*this->ctrl_container)->ptr);
            return ESharedPtr<C>(
                new EPointerControl<C>*(
                    new EPointerControl<C>(
                        (C *)(*this->ctrl_container)->ptr,
                        (*this->ctrl_container)->e_instance_count
                    )
                )
            );
        }
        // explicit shared to weak conversion.
        template<typename W>
        W weak() {
            return W(this->ctrl_container);
        }
};

template<typename T>
class EWeakPtr {
    private:
        EPointerControl<T> ** ctrl_container;
    public:
        EWeakPtr() = default;
        EWeakPtr(EPointerControl<T> ** cc) {
            ctrl_container = cc;
        }
        ~EWeakPtr() {}
        ESharedPtr<T> lock() {
            if (this->ctrl_container == nullptr || *this->ctrl_container == nullptr) {
                return ESharedPtr<T>();
            } else {
                return ESharedPtr<T>(this->ctrl_container);
            }
        }
        // implicit shared to weak conversion.
        EWeakPtr(ESharedPtr<T>& x) {
            ctrl_container = x.ctrl_container;
        }
        EWeakPtr<T>& operator= (ESharedPtr<T>& x) {
            ctrl_container = x.ctrl_container;
            return *this;
        }
};
};

#endif