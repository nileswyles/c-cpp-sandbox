#ifndef WYLESLIBS_CSV_H
#define WYLESLIBS_CSV_H

#include "iostream/iostream.h"
#include "datastructures/datastructures.h"
#include <string>
#include <memory>

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
                for (size_t i = 0; i < this->rows(); i++) {
                    for (size_t j = 0; j < this->columns(); j++) {
                        s += (*this)[i][j];
                        if (i + 1 == this->columns()) {
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
            void read(CSV<std::string> * csv, size_t r_count, MatrixVector<std::string> * header) {
                bool quoted = false;
                uint8_t b;
                size_t r_i = csv->rows() > 0 ? csv->rows() - 1: 0;
                MatrixVector<std::string> r;
                if (header != nullptr) {
                    r_count = 1;
                    r = *header;
                } else {
                    r = (*csv)[r_i];
                }
                std::string current_str;
                while (r_i < r_count) {
                    b = (*io).readByte();
                    if (b == (uint8_t)EOF) {
                        break;
                    }
                    if (!quoted) {
                        if (b == '"') {
                            quoted = true;
                            continue;
                        } else if (b == '\r') {
                            continue;
                        }
                    } else {
                        quoted = false;
                        continue;
                    }
                    if (b == ',' || b == '\n') {
                        r.append(current_str);
                        current_str = "";
                        // end of field
                        if (b == '\n') {
                            size_t new_record_size = (*csv)[r_i].size();
                            if (r_i == 0) {
                                // first iteration
                                record_size = new_record_size;
                            }
                            if (new_record_size == 0 || record_size != new_record_size) {
                                std::runtime_error("Invalid record size.");
                            }
                            // end of record
                            // #containerization
                            record_size = new_record_size;
                            loggerExec(LOGGER_DEBUG_VERBOSE,
                                std::string s;
                                for (size_t i = 0; i < record_size; i++) {
                                    s += (*csv)[r_i][i];
                                    if (i + 1 != record_size) {
                                        s += ",";
                                    }
                                }
                                loggerPrintf(LOGGER_DEBUG_VERBOSE, "RECORD: '%s'\n", s.c_str());
                            );
                            r = (*csv)[++r_i];
                        }
                        continue;
                    } 
                    current_str.push_back((char)b);
                }
            }
        public:
            std::shared_ptr<IOStream> io;
            size_t record_size;

            CSVParser(std::shared_ptr<IOStream> io, char delimeter): io(io), record_size(0) {}
            CSVParser(std::shared_ptr<IOStream> io): CSVParser(io, ',') {}
            ~CSVParser() = default;

            void readDoubles(CSV<double>& csv, size_t r_count) {
                uint8_t b;
                size_t r_i = csv.rows() > 0 ? csv.rows() - 1: 0;
                MatrixVector<double> r = csv[r_i];
                double current_double = 0;
                while (r_i < r_count) {
                    b = (*io).readByte();
                    if (b == (uint8_t)EOF) {
                        break;
                    }
                    if (b == ',' || b == '\n') {
                        r.append(current_double);
                        current_double = 0;
                        // end of field
                        if (b == '\n') {
                            size_t new_record_size = csv[r_i].size();
                            if (r_i == 0) {
                                // first iteration
                                record_size = new_record_size;
                            }
                            if (new_record_size == 0 || record_size != new_record_size) {
                                std::runtime_error("Invalid record size.");
                            }
                            // end of record
                            // #containerization
                            loggerExec(LOGGER_DEBUG_VERBOSE,
                                std::string s;
                                char dec[32];
                                for (size_t i = 0; i < record_size; i++) {
                                    sprintf(dec, "%f", csv[r_i][i]);
                                    s += dec;
                                    if (i + 1 != record_size) {
                                        s += ",";
                                    }
                                }
                                loggerPrintf(LOGGER_DEBUG_VERBOSE, "RECORD: '%s'\n", s.c_str());
                            );
                            // end of record
                            // #containerization
                            r = csv[++r_i];
                        }
                        continue;
                    }
                    size_t dummy_digit_count = 0;
                    if (b == '.') {
                        (*io).readDecimal(current_double, dummy_digit_count);
                    } else {
                        (*io).readNatural(current_double, dummy_digit_count);
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
                    read(&csv, 1, &csv.header);
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