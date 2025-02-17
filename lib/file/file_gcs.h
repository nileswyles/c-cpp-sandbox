#ifndef WYLESLIBS_FILEGCS_H
#define WYLESLIBS_FILEGCS_H

#include "estream/byteestream.h"
#include "file/file.h"
#include "file/stream_factory_gcs.h"

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
        GCSFileManager() = default;
        GCSFileManager(std::string bucket_name): bucket_name(bucket_name) {
            client = google::cloud::storage::Client(google::cloud::Options{}.set<google::cloud::LoggingComponentsOption>({"rpc", "rpc-streams", "auth"}));
            FileManager(
                ESharedPtr<StreamFactory>(
                    std::shared_ptr<StreamFactory>(
                        dynamic_cast<StreamFactory *>(
                            new GCSStreamFactory(client, bucket_name)
                        )
                    )
                )
            );
        }
        ~GCSFileManager() override final = default;

        bool exists(std::string path) override final;
        uint64_t stat(std::string path) override final;

        SharedArray<std::string> list(std::string path) override final;

        void remove(std::string path) override final;
        void move(std::string path, std::string destination_path) override final;
        void copy(std::string path, std::string destination_path) override final;
};
}
#endif