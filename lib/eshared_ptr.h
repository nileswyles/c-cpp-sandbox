#ifndef WYLESLIBS_ESHARED_PTR_H
#define WYLESLIBS_ESHARED_PTR_H

#include "logger.h"
#include "string_format.h"

#include <memory>

#define ESHAREDPTR_GET_PTR(eshared_ptr) eshared_ptr.getPtr(__FILE__, __LINE__)
#define ESHAREDPTR_GET_REF(eshared_ptr) eshared_ptr.getRef(__FILE__, __LINE__)

namespace WylesLibs {

// TODO: In addition to static analysis?
template<typename T>
class ESharedPtr {
    private:
        std::shared_ptr<T> shared_ptr;
    public:
        ESharedPtr() = default;
        ESharedPtr(T * ptr): ESharedPtr(std::shared_ptr<T>(ptr)) {}
        ESharedPtr(std::shared_ptr<T> shared_ptr): shared_ptr(shared_ptr) {}
        ~ESharedPtr() = default;

        T * getPtr(const char * file_name, int line) {
            T * ptr = this->shared_ptr.get();
            if (ptr == nullptr) {
                std::string msg = WylesLibs::format("Attempted to retrieve ptr at: {s}:{d}", file_name, line);
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return ptr;
        }
        T& getRef(const char * file_name, int line) {
            T * ptr = this->shared_ptr.get();
            if (ptr == nullptr) {
                std::string msg = WylesLibs::format("Attempted to retrieve ptr at func: {s}:{d}", file_name, line);
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return *ptr;
        }
        std::weak_ptr<T> getWeak() {
            // TODO: if null and getWeak? should we throw exception? or don't care?
            //      see FileWatcher...
            return this->shared_ptr;
        }
        explicit operator bool() const noexcept {
            return this->shared_ptr.get() != nullptr;
        }
        bool operator!() const noexcept {
            return this->shared_ptr.get() == nullptr;
        }
        bool operator==(const std::shared_ptr<T>& ptr) const noexcept {
            return this->shared_ptr == ptr;
        }
        bool operator!=(const std::shared_ptr<T>& ptr) const noexcept {
            return this->shared_ptr != ptr;
        }
        ESharedPtr<T>& operator=(T * ptr) noexcept {
            this->shared_ptr.reset(ptr);
            return *this;
        }
};

template<typename T, typename L>
static ESharedPtr<L> dynamic_eshared_cast(ESharedPtr<T> ptr) {
    return ESharedPtr<L>(
        std::shared_ptr<L>(
            dynamic_cast<L *>(ESHAREDPTR_GET_PTR(ptr))
        )
    );
}
};

#endif