#ifndef WYLESLIBS_FILES_H
#define WYLESLIBS_FILES_H

#include "estream/istreamestream.h"
#include "file/stream_factory.h"
#include "memory/pointers.h"

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

static void write(ESharedPtr<std::basic_ostream<char>> s_shared, SharedArray<uint8_t> buffer, bool append = false) {
    std::basic_ostream<char> * s = ESHAREDPTR_GET_PTR(s_shared);

    if (true == append) {
        s->seekp(0, std::basic_ostream<char>::end);
    } else {
        // TODO: I think expected behavior here is to overwrite file... if size < current size, this file should end at new size....
        s->seekp(0);
    }
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Writing to file stream:\n%s\n", buffer.toString().c_str());
    s->write((char *)buffer.begin(), buffer.size()); // binary output
    s->flush();
}

static void write(ESharedPtr<std::basic_ostream<char>> s_shared, SharedArray<uint8_t> buffer, size_t offset = 0) {
    std::basic_ostream<char> * s = ESHAREDPTR_GET_PTR(s_shared);

    s->seekp(offset);
    loggerPrintf(LOGGER_DEBUG_VERBOSE, "Writing to file stream:\n%s\n", buffer.toString().c_str());
    s->write((const char *)buffer.begin(), buffer.size()); // binary output
    s->flush();
}

static SharedArray<uint8_t> read(ESharedPtr<IStreamEStream> s_shared, size_t offset = 0, size_t size = SIZE_MAX) {
    IStreamEStream * s = ESHAREDPTR_GET_PTR(s_shared);

    SharedArray<uint8_t> file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (true == s->good()) { //  && false == s->eof(); implied
            uint8_t c = s->get();
            // good and eof are true, false (respectively) at eof character, so ignore.
            if (c != 0xFF) {
                file_data.append(c);
            }
        }
        // if (true == s->fail()) {
        //     throw std::runtime_error("Error occured while reading istream until EOF.");
        // }
    } else {
        file_data = s->readEls(size);
    }
    return file_data;
}

// TODO: refactored to use string type until I can better architect the EStream stuff... need better templating support
static std::string readString(ESharedPtr<IStreamEStream> s_shared, size_t offset = 0, size_t size = SIZE_MAX) {
    IStreamEStream * s = ESHAREDPTR_GET_PTR(s_shared);

    std::string file_data;
    if (offset != 0) {
        s->seekg(offset); // read from absolute position defined by offset
    } // else read from current (relative) position.
    if (size == SIZE_MAX) {
        // read until EOF
        while (true == s->good()) { //  && false == s->eof(); implied
            uint8_t c = s->get();
            // good and eof are true, false (respectively) at eof character, so ignore.
            if (c != 0xFF) {
                file_data += c;
            }
        }
        // if (true == s->fail()) {
        //     throw std::runtime_error("Error occured while reading istream until EOF.");
        // }
    } else {
        file_data = s->readString(size);
    }
    return file_data;
}

// ! IMPORTANT - This must maintain thread safety.
class FileManager {
    protected:
        ESharedPtr<StreamFactory> stream_factory;
    public:
        FileManager(): stream_factory(ESharedPtr<StreamFactory>(new StreamFactory)) {}
        FileManager(ESharedPtr<StreamFactory> stream_factory): stream_factory(stream_factory) {}
        virtual ~FileManager() = default;

        void write(std::string path, SharedArray<uint8_t> buffer, bool append = false) {
            File::write(ESHAREDPTR_GET_PTR(this->stream_factory)->writer(path), buffer, append);

            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            ESHAREDPTR_GET_PTR(this->streams())->removeWriter(path);
        }
        void write(std::string path, SharedArray<uint8_t> buffer, size_t offset = 0) {
            File::write(ESHAREDPTR_GET_PTR(this->stream_factory)->writer(path), buffer, offset);
            // TODO: hopefully it's more like ifstream than istream, doubt it?
            //  That's rather annoying?
            // s->close();
            ESHAREDPTR_GET_PTR(this->streams())->removeWriter(path);
        }
        SharedArray<uint8_t> read(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
            return File::read(
                ESharedPtr<IStreamEStream>(
                    new IStreamEStream(this->stream_factory, path, offset, size)
                ), 
            offset, size);
        }
        std::string readString(std::string path, size_t offset = 0, size_t size = SIZE_MAX) {
            return File::readString(
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