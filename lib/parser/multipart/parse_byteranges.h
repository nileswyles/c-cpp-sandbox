#ifndef WYLESLIBS_PARSE_BYTERANGES_H
#define WYLESLIBS_PARSE_BYTERANGES_H

#include <stdexcept>
#include <string>
#include <map>

#include "server_config.h"
#include "paths.h"

using WylesLibs::Parser;

namespace WylesLibs::Parser::Multipart::ByteRanges {
// lol, still valid for client code...
static void parse(ByteEStream * r, MultipartFile& file) {
    while (1) {
        SharedArray<uint8_t> potential_boundary = r->read("\n"); // read and consume boundary string
        if (potential_boundary.at(0) == '-' && potential_boundary.at(1) == '-') {
            if (potential_boundary.at(potential_boundary.size() - 3) == '-') break;
            // assume type then range for now...
            r->read("\n"); // read and consume content type because who cares.

            // parse byte range...

            //   Content-Range       = range-unit SP
            //   ( range-resp / unsatisfied-range )
            // 
            //   range-resp          = incl-range "/" ( complete-length / "*" )
            //   incl-range          = first-pos "-" last-pos
            //   unsatisfied-range   = "*/" complete-length
            // 
            //   complete-length     = 1*DIGIT

            // Content-Range: bytes 0-50/1024
            r->read(" "); // read and consume Content-Range: |
            r->read(" "); // read and consume bytes |

            double min = 0;
            uint32_t digits = 0;
            r->parseNatural(min, digits);
            if (r->get() != '-') {
                // -50 == 0-50... that's fine...
                // throw exception...
                throw std::runtime_error("Out of bounds error.");
            }

            double max = 0;
            digits = 0;
            r->parseNatural(max, digits);
            if (r->get() != '/') {
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

            // inclusive...
            // [0,0] = 1, [1,0] = 0, [0,1] = 2
            //            [2,0] = -1
            SharedArray<uint8_t> data = r->read(max - min + 1);
            // WylesLibs::File::write(file.getResourcePath(), data);
        } else {
            throw std::runtime_error("Boundary string expected at start or after read.");
        }
    }
}

}
#endif