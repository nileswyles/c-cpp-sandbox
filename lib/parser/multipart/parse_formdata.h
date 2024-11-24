#ifndef WYLESLIBS_PARSE_BYTERANGES_H
#define WYLESLIBS_PARSE_BYTERANGES_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "web/server_config.h"
#include "web/services.h"
#include "parser/multipart/multipart_file.h"
#include "datastructures/array.h"

using namespace WylesLibs;

namespace WylesLibs::Parser::Multipart::FormData {

static constexpr size_t MAX_NUM_FILES_PER_REQUEST = 64;

static void parse(ByteEStream * io, SharedArray<MultipartFile> files, std::unordered_map<std::string, std::string> form_content, ESharedPtr<FileManager> file_manager) {
    // files are uploaded in one request (less overhead - no limit...) hence, timeout in http.cpp.
    bool new_file = true;
    while (1) {
        std::string field_name;
        bool is_file = false;
        MultipartFile file;
        SharedArray<uint8_t> line = io->read("\n"); // read and consume boundary string
        if (line.at(0) == '-' && line.at(1) == '-') {
            if (line[line.size() - 3] == '-') {
                // last boundary string, we are done reading files... 
                break;
            };
            // assume type then range for now...
            io->read(";"); // read and consume content disposition type because who cares.

            is_file = false;
            bool has_name = false;
            for (uint8_t i = 0; i < 2; i++) {
                std::string field = io->read("=").removeBack().toString();
                ReaderTaskExtract extract('"', '"');
                std::string value = io->read(";", &extract).removeBack().toString();
                if (field == "filename") {
                    is_file = true;
                    if (files.size() > MAX_NUM_FILES_PER_REQUEST) {
                        throw std::runtime_error("Stack size... lol");
                    }
                    if (false == files.contains({value})) {
                        file = Service::createMultipartFile(value);
                        files.append(file);
                        new_file = true;
                    } else {
                        file = files[value];
                    }
                } else if (field == "name") {
                    field_name = value;
                    has_name = true;
                }
            }
            if (!has_name) {
                throw std::runtime_error("Field name required.");
            }
            io->read("\n"); // consume new line...
        } else if (field_name != "") {
            if (is_file) {
                file_manager->write(file.getResourcePath(), line, !new_file);
                new_file = false;
            } else {
                form_content[field_name] += line.toString();
            }
        } else {
            throw std::runtime_error("Boundary string expected at start.");
        }
    }
}
}
#endif