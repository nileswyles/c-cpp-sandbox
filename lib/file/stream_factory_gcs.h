#ifndef WYLESLIBS_GCS_STREAM_FACTORY_H
#define WYLESLIBS_GCS_STREAM_FACTORY_H

#include "stream_factory.h"
#include <memory>
#include "pointers.h"
#include <string>

// TODO: compiler flags to not have to rely on google?
#include "client.h"

namespace WylesLibs::File {
class GCSStreamFactory: public StreamFactory {
    private:
        google::cloud::storage::Client client;
        std::string bucket_name;
    public:
        GCSStreamFactory() = default;
        GCSStreamFactory(google::cloud::storage::Client client, std::string bucket_name): client(client), bucket_name(bucket_name), StreamFactory() {}
        ~GCSStreamFactory() override final = default; 
        ESharedPtr<std::basic_istream<char>> reader(std::string path, size_t offset = 0, size_t size = SIZE_MAX) override final;
        ESharedPtr<std::basic_ostream<char>> writer(std::string path) override final;
};
};

#endif