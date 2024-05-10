#include <stdlib.h>
#include <stdio.h>
#include <new>

#include "logger.h"

#include "emalloc.h"

void* operator new(std::size_t sz) {
    logger_printf(LOGGER_DEBUG, "(1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = malloc(sz))
        return ptr;

    throw std::bad_alloc{};
}

void* operator new[](std::size_t sz) {
    logger_printf(LOGGER_DEBUG, "(2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = malloc(sz))
        return ptr;

    throw std::bad_alloc{};
}

void operator delete(void* ptr) noexcept {
    logger_printf(LOGGER_DEBUG, "(3) delete(void*)");
    free(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept {
    logger_printf(LOGGER_DEBUG, "(4) delete(void*, size_t), size = %zu\n", size);
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    logger_printf(LOGGER_DEBUG, "(5) delete[](void* ptr)");
    free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    logger_printf(LOGGER_DEBUG, "(6) delete[](void*, size_t), size = %zu\n", size);
    free(ptr);
}