#include "file/stream_factory.h"
#include "paths.h"

using namespace WylesLibs;

namespace gcs = ::google::cloud::storage;
// ! IMPORTANT - It's important the user understands how the ReadObject functionality works.
//               Normally (size == SIZE_MAX), the ReadObject call returns a stream representing a file on GCS. 
//               Read's are buffered, the entire file is not held in memory and apparently seek functionality is non-existent. 
//
//               This method provides a way of getting a stream representing a range of bytes of a file on GCS.
//               With trade-off of slightly more overhead, you are provided control of chunk sizes and "better" seeking functionality.

//               Additionally, this extra "factory" abstraction is only required because of this ranging functionality.
//               TODO: think about whether it should be it's own function called ranges_reader or something?
std::shared_ptr<std::basic_istream<char>> GCSFileStreamFactory::reader(std::string path, size_t offset, size_t size) {
    std::shared_ptr<std::basic_istream<char>> stream;
    gcs::ObjectReadStream reader;
    // TODO: can I read from offset to end of file? I think so. ReadFromOffset option implies otherwise?
    // TODO: inclusive?
    reader = this->client.ReadObject(this->bucket_name, path, gcs::ReadRange(static_cast<std::int64_t>(offset), static_cast<std::int64_t>(offset + size)));
    if (!reader) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create range reader.");
    }
    // TODO: cast then shared or shared then cast?
    stream = std::dynamic_pointer_cast<std::basic_istream<char>>(
                std::make_shared<gcs::ObjectReadStream>(std::move(reader))
             );
    return stream;
}

std::shared_ptr<std::basic_ostream<char>> GCSFileStreamFactory::writer(std::string path) {
    pthread_mutex_lock(&this->writers_lock);
    if (false == this->writers.contains(path)) {
        return nullptr;
    }
    auto writer = this->client.WriteObject(this->bucket_name, path);
    if (!writer.metadata()) {
        // TODO: log these error messages here..
        throw std::runtime_error("Failed to create writer.");
    }
    std::shared_ptr<std::basic_ostream<char>> w = std::dynamic_pointer_cast<std::basic_ostream<char>>(
                                                      std::make_shared<google::cloud::storage::ObjectWriteStream>(std::move(writer))
                                                  );
    this->writers.insert(path); 
    pthread_mutex_unlock(&this->writers_lock);
    return w;
}