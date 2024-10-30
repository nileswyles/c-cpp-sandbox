#ifndef WYLESLIBS_FILEGCS_H
#define WYLESLIBS_FILEGCS_H

#include "estream/estream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>
#include <fstream>
#include <unordered_map>

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

#include "file.h"

// TODO: compiler flags to not have to rely on google?
#include "google/cloud/storage/client.h"

using namespace WylesLibs;

namespace WylesLibs::File {
class GCSFileManager: public FileManager {
    private:
        std::string bucket_name;
    public:
        google::cloud::storage::Client client;

        GCSFileManager(std::string bucket_name): bucket_name(bucket_name), client(google::cloud::storage::Client()) {};
        ~GCSFileManager() override final = default;

        std::shared_ptr<ReaderEStream> reader(std::string path) override final;
        std::shared_ptr<WriterEStream> writer(std::string path) override final;

        struct stat stat(std::string path) override final;

        SharedArray<std::string> list(std::string path) override final;

        void remove(std::string path) override final;
        void move(std::string path, std::string destination_path) override final;
        void copy(std::string path, std::string destination_path) override final;
};
}
#endif