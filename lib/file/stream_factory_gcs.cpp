#include "file/stream_factory_gcs.h"
#include "paths.h"

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_STREAM_FACTORY_GCS
#define LOGGER_LEVEL_STREAM_FACTORY_GCS GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_STREAM_FACTORY_GCS
#define LOGGER_STREAM_FACTORY_GCS 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_STREAM_FACTORY_GCS

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_STREAM_FACTORY_GCS
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::File;

namespace gcs = ::google::cloud::storage;
// ! IMPORTANT - It's important the user understands how the ReadObject functionality works.
//               Normally (size == SIZE_MAX), the ReadObject call returns a stream representing a file on GCS. 
//               Read's are buffered, the entire file is not held in memory and apparently seek functionality is non-existent. 
//
//               This method provides a way of getting a stream representing a range of bytes of a file on GCS.
//               With trade-off of slightly more overhead, you are provided control of chunk sizes and "better" seeking functionality.

//               Additionally, this extra "factory" abstraction is only required because of this ranging functionality.
//               TODO: think about whether it should be it's own function called ranges_reader or something?
ESharedPtr<std::basic_istream<char>> GCSStreamFactory::reader(std::string path, size_t offset, size_t size) {
    // TODO: can I read from offset to end of file? I think so. ReadFromOffset option implies otherwise?
    // TODO: inclusive?
    gcs::ObjectReadStream reader = this->client.ReadObject(this->bucket_name, path, gcs::ReadRange(static_cast<std::int64_t>(offset), static_cast<std::int64_t>(offset + size)));
    if (false == reader.IsOpen() || false == reader.good()) {
        std::string msg = "Unable to open file at path: " + path + " for reading.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    return ESharedPtr<std::basic_istream<char>>(
        std::shared_ptr<std::basic_istream<char>>(
            dynamic_cast<std::basic_istream<char> *>(
                new gcs::ObjectReadStream(std::move(reader))
            )
        )
    );
}

ESharedPtr<std::basic_ostream<char>> GCSStreamFactory::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    gcs::ObjectWriteStream writer = this->client.WriteObject(this->bucket_name, path);
    if (false == writer.IsOpen() || false == writer.good()) {
        std::string msg = "Unable to open file at path: " + path + " for writing.\n";
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    // TODO: std::move?
    return ESharedPtr<std::basic_ostream<char>>(
        std::shared_ptr<std::basic_ostream<char>>(
            dynamic_cast<std::basic_ostream<char> *>(
                new gcs::ObjectWriteStream(std::move(writer))
            )
        )
    );
}