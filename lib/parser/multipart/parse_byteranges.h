#ifndef WYLESLIBS_PARSE_BYTERANGES_H
#define WYLESLIBS_PARSE_BYTERANGES_H

#include <stdexcept>
#include <string>
#include <map>

#include "server_config.h"
#include "paths.h"

using WylesLibs::Parser;

namespace WylesLibs {

class MultipartFile {
    private:
        std::string resource_root;
    public:
        std::string id;
        std::string name;

        MultipartFile(): resource_root("./") {
            throw std::runtime_error("What even are you doing!");
        }

        MultipartFile(ServerConfig config, std::string id, std::string name) {
            resource_root = config.resources_root;
            id = id;
            name = name;
            // hmm.. yeah so my initial approach of using an in-memory database for now was right
        }

        std::string getResourcePath() {
            return Paths::join(this->resource_root, this->id);
        }
};
}

namespace WylesLibs::Parser::Multipart::ByteRanges {
static void parse(Reader * r, MultipartFile& file) {
    // can read bytes specified in range
    //   or
    //  can read until next boundary line

    // how strict do I want to be?
    // revisit specification...

    while (1) {
        Array<uint8_t> potential_boundary = r->readUntil("\n"); // read and consume boundary string
        if (potential_boundary.buf[0] == '-' && potential_boundary.buf[1] == '-') {
            if (potential_boundary.back() == '-' && potential_boundary.buf[potential_boundary.size() - 2] == '-') break;
            // assume type then range for now...
            r->readUntil("\n"); // read and consume content type because who cares.

            // parse byte range...
            // Content-Range: bytes 0-50/1024
            r->readUntil(" "); // read and consume Content-Range: |
            r->readUntil(" "); // read and consume bytes |

            double min = 0;
            uint32_t digits = 0;
            parseNatural(r, min, digits);
            if (r->readByte() != '-') {
                // -50 == 0-50... that's fine...
                // throw exception...
                throw std::runtime_error("Out of bounds error.");
            }

            double max = 0;
            digits = 0;
            parseNatural(r, max, digits);
            if (r->readByte() != '/') {
                // 0- == 0-0... that's fine...
                // throw exception...
                throw std::runtime_error("Out of bounds error.");
            }

            size_t size = max - min;
            if (size < 1) {
                // throw exception.
                // -/ == 0-0, that's also fine...
                throw std::runtime_error("Out of bounds error.");
            }
            Array<uint8_t> data = r->readBytes(max - min);
            WylesLibs::File::writeFile(file.getResourcePath(), data);
        } else {
            throw std::runtime_error("Boundary string expected at start or after read.");
        }
    }

    // bool init = false;
    // while (1) {
    //     Array<uint8_t> potential_boundary = r->readUntil("\n"); // read and consume boundary string
    //     if (potential_boundary.buf[0] == '-' && potential_boundary.buf[1] == '-') {
    //         if (potential_boundary.back() == '-' && potential_boundary.buf[potential_boundary.size() - 2] == '-') break;
    //         init = true;
    //         r->readUntil("\n"); // read and consume content type because who cares.
    //         r->readUntil("\n"); // read and consume content range because also who cares.
    //     } else if (!init) {
    //         throw std::runtime_error("Request content must start with boundary string.");
    //     } else {
    //         WylesLibs::File::writeFile(file.getResourcePath(), potential_boundary);
    //     }
    // }
}

}
#endif