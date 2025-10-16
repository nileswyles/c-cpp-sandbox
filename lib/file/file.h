#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "istreamestream.h"
#include "io.h"
#include "array.h"
#include "stream_factory.h"
#include "pointers.h"

#include <ios>
#include <string>
#include <memory>

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_FILE
#define LOGGER_LEVEL_FILE GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_FILE
#define LOGGER_FILE 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_FILE

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_FILE
#include "logger.h"

using namespace WylesLibs;

namespace WylesLibs::File {
    
    static const std::string stream_error_msg("The stream provided has an error.");
    
    // ! IMPORTANT - This must maintain thread safety.
    class FileManager {
        protected:
            ESharedPtr<StreamFactory> stream_factory;
        public:
            FileManager(): stream_factory(ESharedPtr<StreamFactory>(new StreamFactory)) {}
            FileManager(ESharedPtr<StreamFactory> stream_factory): stream_factory(stream_factory) {}
            virtual ~FileManager() = default;
    
            template<typename T>
            void write(std::string path, T buffer, bool append = false) {
                WylesLibs::write<T>(ESHAREDPTR_GET_PTR(this->stream_factory)->writer(path), buffer, append);
            
                // TODO: hopefully it's more like ifstream than istream, doubt it?
                //  That's rather annoying?
                // s->close();
                ESHAREDPTR_GET_PTR(this->streams())->removeWriter(path);
            }
            template<typename T>
            void write(std::string path, T buffer, size_t offset = 0) {
                WylesLibs::write<T>(ESHAREDPTR_GET_PTR(this->stream_factory)->writer(path), buffer, offset);
                // TODO: hopefully it's more like ifstream than istream, doubt it?
                //  That's rather annoying?
                // s->close();
                ESHAREDPTR_GET_PTR(this->streams())->removeWriter(path);
            }
            SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
                return WylesLibs::read(
                    ESharedPtr<IStreamEStream>(
                        new IStreamEStream(this->stream_factory, path, offset, size)
                    ), 
                offset, size);
            }
            std::string readString(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
                return WylesLibs::readString(
                    ESharedPtr<IStreamEStream>(
                        new IStreamEStream(this->stream_factory, path, offset, size)
                    ), 
                offset, size);
            }
            ESharedPtr<StreamFactory> streams() {
                return this->stream_factory;
            }
        
            virtual bool exists(std::string path);
            virtual uint64_t stat(std::string path);
            virtual SharedArray<std::string> list(std::string path);
        
            virtual void remove(std::string path);
            virtual void move(std::string path, std::string destination_path);
            virtual void copy(std::string path, std::string destination_path);
    };
}
#endif