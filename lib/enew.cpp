#include <stdlib.h>
#include <stdio.h>

#include "emalloc.h"

void* operator new(std::size_t sz) {
    printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = malloc(sz))
        return ptr;

    return NULL;
    // throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void* operator new[](std::size_t sz) {
    printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = malloc(sz))
        return ptr;

    return NULL;
    // throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void operator delete(void* ptr) noexcept {
    printf("3) delete(void*)");
    free(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept {
    printf("4) delete(void*, size_t), size = %zu\n", size);
    free(ptr);
}

void operator delete[](void* ptr) noexcept {
    printf("5) delete[](void* ptr)");
    free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    printf("6) delete[](void*, size_t), size = %zu\n", size);
    free(ptr);
}