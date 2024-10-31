#ifndef WYLESLIBS_ESTREAM_FACTORY_H
#define WYLESLIBS_ESTREAM_FACTORY_H

#include <set>
#include <memory>
#include <string>

// TODO: compiler flags to not have to rely on google?
#include "google/cloud/storage/client.h"

// TODO: so this is no longer estream stuff...
namespace WylesLibs {
class FileStreamFactory {
    protected:
        std::shared_ptr<FileStreamFactory> this_shared;
        std::set<std::string> writers;
        pthread_mutex_t writers_lock;
    public:
        FileStreamFactory() {
            pthread_mutex_init(&writers_lock, nullptr);
        };
        virtual std::shared_ptr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX);
        virtual std::shared_ptr<std::basic_ostream<char>> writer(std::string path);
        // ! IMPORTANT - implementation should call this function when done with writer...
        //      is there a better way? yeah, maybe different ostream type with shared_ptr to this stuff... that removes when close is called...? too complicated?
        virtual void removeWriter(std::string path);
};

class GCSFileStreamFactory: public FileStreamFactory {
    private:
        google::cloud::storage::Client client;
    public:
        GCSFileStreamFactory(google::cloud::storage::Client client): client(client), FileStreamFactory() {}
        std::shared_ptr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX) override final;
        std::shared_ptr<std::basic_ostream<char>> writer(std::string path) override final;
};
};

#endif