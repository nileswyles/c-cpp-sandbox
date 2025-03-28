#ifndef WYLESLIBS_ESTREAM_FACTORY_H
#define WYLESLIBS_ESTREAM_FACTORY_H

#include <memory>
#include <string>

#include "datastructures/array.h"
#include "memory/pointers.h"

#if defined(_MSC_VER)
#include <mutex>
#else
#include <pthread.h>
#endif

namespace WylesLibs::File {
// ! IMPORTANT - This must maintain thread safety.
class StreamFactory {
    protected:
        SharedArray<std::string> writers;
        #if defined(_MSC_VER)
        std::mutex writers_lock;
        #else
        pthread_mutex_t writers_lock;
        #endif
    public:
        StreamFactory() {
            #if defined(_MSC_VER)
            #else
            pthread_mutex_init(&writers_lock, nullptr);
            #endif
        }
        virtual ~StreamFactory() = default;
        virtual ESharedPtr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX);
        virtual ESharedPtr<std::basic_ostream<char>> writer(std::string path);
        // ! IMPORTANT - implementation should call this function when done with writer...
        //      is there a better way? yeah, maybe different ostream type with shared_ptr to this stuff... that removes when close is called...? too complicated?
        virtual void removeWriter(std::string path);
};
};

#endif