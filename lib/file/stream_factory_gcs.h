#ifndef WYLESLIBS_ESTREAM_FACTORY_H
#define WYLESLIBS_ESTREAM_FACTORY_H

#include "file/stream_factory.h"
#include <memory>
#include <string>

// TODO: compiler flags to not have to rely on google?
#include "google/cloud/storage/client.h"

// TODO: so this is no longer estream stuff...
namespace WylesLibs {
class GCSFileStreamFactory: public FileStreamFactory {
    private:
        google::cloud::storage::Client client;
        std::string bucket_name;
    public:
        GCSFileStreamFactory(google::cloud::storage::Client client, std::string bucket_name): client(client), bucket_name(bucket_name), FileStreamFactory() {}
        std::shared_ptr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX) override final;
        std::shared_ptr<std::basic_ostream<char>> writer(std::string path) override final;
};
};

#endif