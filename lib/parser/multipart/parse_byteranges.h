#ifndef WYLESLIBS_PARSE_BYTERANGES_H
#define WYLESLIBS_PARSE_BYTERANGES_H

#include <stdexcept>
#include <string>
#include <map>

#include "server_config.h"
#include "paths.h"

namespace WylesLibs {

class MultipartFile {
    private:
        std::string resource_root;
    public:
        std::string resource_uuid;
        std::string name;

        MultipartFile(): resource_root("./") {
            throw std::runtime_error("What even are you doing!");
        }

        MultipartFile(ServerConfig config) {
            resource_root = config.resources_root;
            // database information 
            // new resource_uuid, different from boundary and database id...
            //  resource_uuid is name of local file.
            //  name of actual file stored here (and in database, likely postgresql, because featureset)...

            // hmmm.. yeah but on second thought sometimes coupling is unavoidable? so why not more closely work with database system here?
            //  abstractions.... 
            //  hmm.... resource_uuid can be primary key of database table... and name of local file.

            //  this is very cat-like/delegation/ma-like lol I'm confused? or so they say at least?
            //  teamwork!
            //  we all we got

            // similarly, supporting all database systems is very ma-like lol? 
            //  perspective!

            // you know what just toss those categories == i (== all).
            //  logical and sane conclusion?

            //  idk more confusion... but there might be value as a form of communication and that's it.

            // know yourself, i guess is moral of the story :)...

            // alright, back to this...

            // connecting to psql... libpq...
             
        }

        std::string getResourcePath() {
            return Paths::join(this->resource_root, this->resource_uuid);
        }
};
}

namespace WylesLibs::Parser::Multipart::ByteRanges {
static void parse(Reader * r, MultipartFile& file) {

}

}
#endif