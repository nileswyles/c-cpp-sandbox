#ifndef WYLESLIBS_FILES3_H
#define WYLESLIBS_FILES3_H

#include "estream/estream.h"
#include "file.h"

#include <string>

#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {
class S3FileManager: public FileManager {
    public:
        S3FileManager() = default;
        ~S3FileManager() override final = default;

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