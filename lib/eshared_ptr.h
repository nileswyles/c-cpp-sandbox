#ifndef WYLESLIBS_ESHARED_PTR_H
#define WYLESLIBS_ESHARED_PTR_H

#include "logger.h"
#include "string_format.h"

#include <memory>

namespace WylesLibs {
// TODO: In addition to static analysis?
template<typename T>
class ESharedPtr {
    private:
        std::shared_ptr<T> shared_ptr;
    public:
        ESharedPtr() = default;
        ESharedPtr(std::shared_ptr<T> shared_ptr): shared_ptr(shared_ptr) {}
        ~ESharedPtr() = default;

        T * getPtr(std::string func_name) {
            T * ptr = this->shared_ptr.get();
            if (ptr == nullptr) {
                std::string msg = WylesLibs::format("Attempted to retrieve invalid ptr at: {s}", func_name);
                loggerPrintf(LOGGER_INFO, "Exception: %s\n", msg.c_str());
                throw std::runtime_error(msg);
            }
            return ptr;
        }
        T& get(std::string func_name) {
            T * ptr = this->shared_ptr.get();
            if (ptr == nullptr) {
                std::string msg = WylesLibs::format("Attempted to retrieve invalid ptr at func: {s}", func_name);
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
        ESharedPtr<T>& operator=(const std::shared_ptr<T>& ptr) noexcept {
            this->shared_ptr = ptr;
            return *this;
        }
};
};

#endif