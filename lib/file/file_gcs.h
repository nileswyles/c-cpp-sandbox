#ifndef WYLESLIBS_FILEGCS_H
#define WYLESLIBS_FILEGCS_H

#include "estream/estream.h"
#include "file/file.h"

#include <string>
#include <fstream>
#include <unordered_map>

// TODO: compiler flags to not have to rely on google?
#include "google/cloud/storage/client.h"

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {
class GCSFileManager: public FileManager {
    private:
        std::string bucket_name;
        google::cloud::storage::Client client;
    public:
        GCSFileManager(std::string bucket_name): bucket_name(bucket_name), 
                                                 client(google::cloud::storage::Client()),
                                                 FileManager(std::dynamic_pointer_cast<FileStreamFactory>(std::make_shared<GCSFileStreamFactory>(client, bucket_name))) {

                                                 };
        ~GCSFileManager() override final = default;

        uint64_t stat(std::string path) override final;

        SharedArray<std::string> list(std::string path) override final;

        void remove(std::string path) override final;
        void move(std::string path, std::string destination_path) override final;
        void copy(std::string path, std::string destination_path) override final;
};
}
#endif