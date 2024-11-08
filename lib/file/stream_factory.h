#ifndef WYLESLIBS_ESTREAM_FACTORY_H
#define WYLESLIBS_ESTREAM_FACTORY_H

#include <set>
#include <memory>
#include <string>

namespace WylesLibs::File {
class StreamFactory {
    protected:
        std::shared_ptr<StreamFactory> this_shared;
        std::set<std::string> writers;
        pthread_mutex_t writers_lock;
    public:
        StreamFactory() {
            pthread_mutex_init(&writers_lock, nullptr);
        }
        virtual ~StreamFactory() = default;
        virtual std::shared_ptr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX);
        virtual std::shared_ptr<std::basic_ostream<char>> writer(std::string path);
        // ! IMPORTANT - implementation should call this function when done with writer...
        //      is there a better way? yeah, maybe different ostream type with shared_ptr to this stuff... that removes when close is called...? too complicated?
        virtual void removeWriter(std::string path);
};
};

#endif