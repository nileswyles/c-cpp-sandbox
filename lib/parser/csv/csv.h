#ifndef WYLESLIBS_CSV_H
#define WYLESLIBS_CSV_H

#include "iostream/iostream.h"
#include "string_utils.h"
#include "datastructures/datastructures.h"
#include <string>
#include <memory>

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
    template<typename T>
    class CSV: public Matrix<T> {
        public:
            MatrixVector<std::string> header;
            CSV() = default;
            ~CSV() override final = default;
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
                // they hating on me, hating on me, kisss kiss kiss kiss
                for (size_t y = 0; y < this->rows() - 1; y++) {
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
    class CSVParser {
        private:
            bool assertCSVNoHeader(CSV<std::string> * csv, MatrixVector<std::string> * header) {
                return csv != nullptr && header == nullptr;
            }
            bool assertHeaderNoCSV(CSV<std::string> * csv, MatrixVector<std::string> * header) {
                return csv == nullptr && header != nullptr;
            }
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
            bool handleFieldDelimeter(CSV<std::string> * csv, std::string& current_str, uint8_t b, MatrixVector<std::string> * header, size_t& r_i) {
                bool result = false;
                MatrixVector<std::string> r;
                if (true == assertHeaderNoCSV(csv, header)) {
                    r = *header;
                } else {
                    r = (*csv)[r_i];
                }
                if (b == separator || b == '\n') {
                    r.append(current_str);
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "CURRENT STR: '%s', '%s'\n", current_str.c_str(), r[0].c_str());
                    current_str = "";
                    // end of field
                    if ('\n' == b) {
                        processRecord(r);
                        // #containerization
                        if (true == assertCSVNoHeader(csv, header)) {
                            // creates new row...
                            r = (*csv)[++r_i];
                        }
                    }
                    result = true;
                } 
                return result;
            }
            bool handleFieldDelimeter(CSV<double> * csv, double& current_double, uint8_t b, size_t& r_i) {
                bool result = false;
                // TODO: no need for explicit null check because references?
                MatrixVector<double> r = (*csv)[r_i];
                if (this->separator == b || '\n' == b) {
                    r.append(current_double);
                    current_double = 0;
                    if ('\n' == b) {
                        processRecord(r);
                        // TODO: better syntax for this...
                        //      function to create and return new element?
                        r = (*csv)[++r_i];
                    }
                    result = true;
                } 
                return result;
            }
            bool handleQuotes(CSV<std::string> * csv, bool& quoted, std::string current_str, uint8_t b) {
                bool result = false;
                if (csv == nullptr) return result;
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
                    uint8_t peeked = (*this->io).peekByte();
                    if (peeked == '"') {
                        (*this->io).readByte();
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
            void read(CSV<std::string> * csv, size_t r_count, MatrixVector<std::string> * header) {
                bool quoted = false;
                uint8_t b;
                size_t r_i = 0;
                std::string current_str;
                while (r_i < r_count) {
                    b = (*this->io).readByte();
                    // TODO: this requires '\n' at end of last record... which might be okay... but good to think about...
                    if (b == (uint8_t)EOF) {
                        break;
                    }
                    // handle quotes
                    if (true == handleQuotes(csv, quoted, current_str, b)) {
                        continue;
                    }
                    // handle field delimeter
                    if (true == handleFieldDelimeter(csv, current_str, b, header, r_i)) {
                        if (true == assertCSVNoHeader(csv, header)) {
                            // point to next row if end of record
                            continue;
                        } else if (true == assertHeaderNoCSV(csv, header)) {
                            // TODO: 
                            // LOL - yeah this is absurd...
                            if ('\n' == b) {
                                break;
                            } else if (this->separator == b) {
                                continue;
                            }
                        }
                    }
                    current_str.push_back((char)b);
                }
            }
        public:
            std::shared_ptr<IOStream> io;
            size_t record_size;
            char separator;

            CSVParser(std::shared_ptr<IOStream> io, char separator): io(io), separator(separator), record_size(0) {
                if (separator == '.') {
                    throw std::runtime_error("Periods aren't allowed as CSV separator.");
                }
            }
            CSVParser(std::shared_ptr<IOStream> io): CSVParser(io, ',') {}
            ~CSVParser() = default;
            void reset() {
                this->record_size = 0;
            }
            void readDoubles(CSV<double>& csv, size_t r_count) {
                uint8_t b;
                size_t r_i = 0;
                MatrixVector<double> r = csv[r_i];
                double current_double = 0;
                while (r_i < r_count) {
                    b = (*this->io).peekByte();
                    // handle field delimeter
                    if (true == handleFieldDelimeter(&csv, current_double, b, r_i)) {
                        (*this->io).readByte();
                        continue;
                    }
                    // handle numbers delimeter
                    size_t dummy_digit_count = 0;
                    if (isDigit(b)) {
                        (*this->io).readNatural(current_double, dummy_digit_count);
                        if ((*this->io).peekByte() == '.') {
                            (*this->io).readByte();
                            (*this->io).readDecimal(current_double, dummy_digit_count);
                        }
                        continue;
                        // buffer should be at non-number-digit
                    } else if (b == (uint8_t)EOF) {
                        (*this->io).readByte();
                        break;
                    } else {
                        throw std::runtime_error("Non-digit character found in CSV data.");
                    }
                }
            }
            CSV<double> readDoubles(bool has_header) {
                CSV<double> csv;
                if (true == has_header) {
                    read(nullptr, 1, &csv.header);
                }
                readDoubles(csv, SIZE_MAX);
                return csv;
            }
            CSV<std::string> read(bool has_header) {
                CSV<std::string> csv;
                if (true == has_header) {
                    read(nullptr, 1, &csv.header);
                }
                read(&csv, SIZE_MAX, nullptr);
                return csv;
            }
            void read(CSV<std::string>& csv, size_t r_count) {
                read(&csv, r_count, nullptr);
            }
    };
};

#endif