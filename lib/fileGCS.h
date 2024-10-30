#ifndef WYLESLIBS_FILES3_H
#define WYLESLIBS_FILES3_H

#include "iostream/iostream.h"

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

#include "google/cloud/storage/client.h"

using namespace WylesLibs;

namespace WylesLibs::File {
class GCSReader {
    public:
        std::string path;
        google::cloud::storage::ObjectReadStream reader;
        pthread_mutex_t lock;
        size_t bytes_read;
        GCSReader(google::cloud::storage::ObjectReadStream reader): reader(reader) {}
        ~GCSReader() {}
};
class GCSWriter {
    public:
        std::string path;
        google::cloud::storage::ObjectWriteStream writer;
        pthread_mutex_t lock;
        size_t bytes_written;
        GCSWriter(google::cloud::storage::ObjectWriteStream writer): writer(writer), bytes_written(0) {}
        ~GCSWriter() {}
};
class GCSFileManager: public FileManager {
    private:
        std::string bucket_name;
        GCSReader getReader(std::string path);
        GCSWriter getWriter(std::string path);
    public:
        google::cloud::storage::Client client;
        // uuid, GCSReader
        std::unordered_map<std::string, GCSReader> reader_map;
        pthread_mutex_t reader_lock;
        // uuid, GCSWriter
        std::unordered_map<std::string, GCSWriter> writer_map;
        pthread_mutex_t writer_lock;

        GCSFileManager(std::string bucket_name): bucket_name(bucket_name), client(google::cloud::storage::Client()) {};
        ~GCSFileManager() override final = default;

        std::shared_ptr<std::istream> read(std::string path) override final;

        void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0) override final;
        void write(std::string path, SharedArray<uint8_t> buffer, bool append = false) override final;

        // probably never needed but why not
        struct stat stat(std::string path) override final;

        SharedArray<std::string> list(std::string path) override final;

        void remove(std::string path) override final;
        void move(std::string path, std::string destination_path) override final;
        void copy(std::string path, std::string destination_path) override final;
};
}
#endif