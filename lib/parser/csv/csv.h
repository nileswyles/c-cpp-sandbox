#ifndef WYLESLIBS_CSV_H
#define WYLESLIBS_CSV_H

#include "estream/byteestream.h"

#include <string>
#include <memory>
#include "eshared_ptr.h"
#include <unistd.h>
#include <fcntl.h>

#include "string_utils.h"
#include "datastructures/datastructures.h"

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_CSV
#define LOGGER_LEVEL_CSV GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_CSV
#define LOGGER_CSV 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_CSV

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_CSV
#include "logger.h"

namespace WylesLibs {

    // @

    template<typename T>
    class CSV: public Matrix<T> {
        private:
            ESharedPtr<ByteEStream> io;
            size_t record_size;
            char separator;
            int fd;
            void updateAndCheckRecordSize(size_t new_record_size) {
                if (0 == this->record_size) {
                    // first iteration
                    this->record_size = new_record_size;
                }
                if (0 == new_record_size || this->record_size != new_record_size) {
                    throw std::runtime_error("Invalid record size.");
                }
            }
            void processRecord(MatrixVector<std::string>& r) {
                updateAndCheckRecordSize(r.size());
                // end of record
                loggerExec(LOGGER_DEBUG_VERBOSE,
                    std::string s;
                    for (size_t i = 0; i < record_size; i++) {
                        s += r[i];
                        if (i + 1 != record_size) {
                            s += ",";
                        }
                    }
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "RECORD: '%s'\n", s.c_str());
                );
                // end of record
            }
            void processRecord(MatrixVector<double>& r) {
                updateAndCheckRecordSize(r.size());
                // end of record
                loggerExec(LOGGER_DEBUG_VERBOSE,
                    std::string s;
                    char dec[32];
                    for (size_t i = 0; i < record_size; i++) {
                        sprintf(dec, "%f", r[i]);
                        s += dec;
                        if (i + 1 != record_size) {
                            s += ",";
                        }
                    }
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "RECORD: '%s'\n", s.c_str());
                );
                // end of record
            }
            bool handleFieldDelimeter(std::string& current_str, uint8_t b, size_t& r_i, size_t& f_i, bool process_header) {
                bool result = false;
                MatrixVector<std::string> r;
                if (true == process_header) {
                    r = this->header;
                } else {
                    r = (*this)[r_i];
                }
                if (b == separator || b == '\n') {
                    r[f_i++] = current_str;
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "CURRENT STR: '%s', record[0]: '%s'\n", current_str.c_str(), r[0].c_str());
                    current_str = "";
                    // end of field
                    if ('\n' == b) {
                        processRecord(r);
                        // #containerization
                        ++r_i;
                        f_i = 0;
                    }
                    result = true;
                } 
                return result;
            }
            bool handleFieldDelimeter(double& current_double, uint8_t b, size_t& r_i, size_t& f_i) {
                bool result = false;
                // TODO: no need for explicit null check because references?
                MatrixVector<double> r = (*this)[r_i];
                if (this->separator == b || '\n' == b) {
                    r[f_i++] = current_double;
                    current_double = 0;
                    if ('\n' == b) {
                        processRecord(r);
                        // #containerization
                        // TODO: better syntax for this...
                        //      function to create and return new element?
                        ++r_i;
                        f_i = 0;
                    }
                    result = true;
                } 
                return result;
            }
            bool handleQuotes(bool& quoted, std::string current_str, uint8_t b) {
                bool result = false;
                if (!quoted) {
                    if ('"' == b) {
                        if (current_str.size() != 0) {
                            throw std::runtime_error("Quoted string cannot be preceded by a non-delimiting character.");
                        }
                        quoted = true;
                        result = true;
                    } else if ('\r' == b) {
                        result = true;
                    }
                } else if (quoted && '"' == b){
                    // peak
                    uint8_t peeked = this->io.get(__func__).peek();
                    if (peeked == '"') {
                        this->io.get(__func__).get();
                    } else if (true == (peeked == this->separator || '\n' == peeked)){
                        // end of quoted string....
                        quoted = false;
                        result = true;
                    } else {
                        throw std::runtime_error("Quoted string cannot be followed by a non-delimiting character.");
                    }
                }
                return result;
            }
            // TODO: this bs because member function specialization doesn't work and don't want to do same as array, for reasons.
            void read(std::string& type, size_t r_count, bool process_header) {
                bool quoted = false;
                uint8_t b;
                size_t r_i = 0;
                size_t f_i = 0;
                std::string current_str;
                while (r_i < r_count) {
                    b = this->io.get(__func__).get();
                    // TODO: this requires '\n' at end of last record... which might be okay... but good to think about...
                    if (b == (uint8_t)EOF) {
                        break;
                    }
                    // handle quotes
                    if (true == handleQuotes(quoted, current_str, b)) {
                        continue;
                    }
                    // handle field delimeter
                    if (true == handleFieldDelimeter(current_str, b, r_i, f_i, process_header)) {
                        continue;
                    }
                    current_str.push_back((char)b);
                }
            }
            void read(double& type, size_t r_count, bool process_header) {
                uint8_t b;
                size_t r_i = 0;
                size_t f_i = 0;
                double current_double = 0;
                while (r_i < r_count) {
                    b = this->io.get(__func__).peek();
                    // handle field delimeter
                    if (true == handleFieldDelimeter(current_double, b, r_i, f_i)) {
                        this->io.get(__func__).get();
                        continue;
                    }
                    // handle numbers delimeter
                    size_t dummy_digit_count = 0;
                    // TODO: see ByteEStream TODO
                    if (isDigit(b)) {
                        current_double = this->io.get(__func__).readDecimal();
                        continue;
                        // buffer should be at non-numeric
                    } else if (b == (uint8_t)EOF) {
                       this->io.get(__func__).get();
                        break;
                    } else {
                        throw std::runtime_error("Non-digit character found in CSV data.");
                    }
                }
            }
        public:
            MatrixVector<std::string> header;

            CSV(ESharedPtr<ByteEStream> io): CSV(io, ',') {}
            CSV(ESharedPtr<ByteEStream> io, char separator): CSV(io, separator, -1) {}
            CSV(ESharedPtr<ByteEStream> io, char separator, int fd): io(io), separator(separator), record_size(0), fd(fd) {
                if (separator == '.') {
                    throw std::runtime_error("Periods aren't allowed as CSV separator.");
                }
            }
            ~CSV() = default;

            void reset() {
                this->record_size = 0;
            }
            void read(bool has_header) {
                // TODO: this doesn't make much sense, user can just create new instance?
                //  actually, no, since it's coupled now, more performance to allow reset (and reconfiguring ByteEStream (new))
                //  TBC
                this->reset();
                T dumb_function_selector;
                if (true == has_header) {
                    read(dumb_function_selector, 1, true);
                }
                // read until EOF...
                read(dumb_function_selector, SIZE_MAX, false);
            }
            void read(size_t r_count) {
                T dumb_function_selector;
                read(dumb_function_selector, r_count, false);
            }
            MatrixVector<T>& operator[] (const size_t pos) {
                return this->matrix[pos];
            }
            std::string toString() {
                // assuming T == std::string for now...
                std::string s;
                for (size_t i = 0; i < this->header.size(); i++) {
                    s += this->header[i];
                    if (i + 1 == this->header.size()) {
                        s += "\n";
                    } else {
                        s += ",";
                    }
                }
                for (size_t y = 0; y < this->rows(); y++) {
                    for (size_t x = 0; x < this->columns(); x++) {
                        s += (*this)[y][x];
                        if (x + 1 == this->columns()) {
                            s += "\n";
                        } else {
                            s += ",";
                        }
                    }
                }
                return s;
            }
    };
};

#endif