#ifndef WYLESLIBS_PARSE_BYTERANGES_H
#define WYLESLIBS_PARSE_BYTERANGES_H

#include <stdexcept>
#include <string>
#include <unordered_map>

#include "web/server_config.h"
#include "parser/multipart/multipart_file.h"
#include "datastructures/array.h"
#include "web/services.h"

using namespace WylesLibs;

namespace WylesLibs::Parser::Multipart::FormData {
//
static void parse(IOStream * io, SharedArray<MultipartFile> files, unordered_map<std::string, std::string> form_content) {
    while (1) {
        std::string field_name;
        bool is_file = false;
        bool new_file = true;
        MultipartFile file;
        SharedArray<uint8_t> line = io->readUntil("\n"); // read and consume boundary string
        if (line.buf()[0] == '-' && line.buf()[1] == '-') {
            if (line.buf()[line.size() - 3] == '-') break;
            // assume type then range for now...
            io->readUntil(";"); // read and consume content disposition type because who cares.

            is_file = false;
            bool has_name = false;
            for (uint8_t i = 0; i < 2; i++) {
                std::string field = io->readUntil("=").removeBack().toString();
                ReaderTaskExtract extract('"', '"');
                std::string value = io->readUntil(";", &extract).removeBack().toString();
                if (field == "filename") {
                    is_file = true;
                    file = Service::createMultipartFile(value);
                    new_file = true;
                } else if (field == "name") {
                    field_name = value;
                    has_name = true;
                }
            }
            if (!has_name) {
                throw std::runtime_error("Field name required.");
            }
            io->readUntil("\n"); // consume new line...
        } else if (field_name != "") {
            if (is_file) {
                WylesLibs::File::write(file.getResourcePath(), line, !new_file);
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