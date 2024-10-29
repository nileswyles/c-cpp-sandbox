#ifndef WYLESLIBS_FILES3_H
#define WYLESLIBS_FILES3_H

#include "iostream/iostream.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>

#include <fstream>
#include <sys/stat.h>


#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE
#include "logger.h"

#include "file.h"

using namespace WylesLibs;

namespace WylesLibs::File {
class S3FileManager: public FileManager {
    public:
        S3FileManager() = default;
        ~S3FileManager() override final = default;

        SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) override final;

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