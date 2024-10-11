#ifndef WYLESLIBS_CSV_H
#define WYLESLIBS_CSV_H

#include "iostream/iostream.h"
#include "datastructures/datastructures.h"
#include <string>
#include <memory>

namespace WylesLibs {
    template<typename T>
    class CSV: protected Matrix<T> {
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
                uint8_t b = (*io).readByte();
                size_t r_i = 0;
                size_t f_i = 0;
                MatrixVector<std::string> r;
                if (header != nullptr) {
                    r_count = 1;
                    r = *header;
                } else {
                    r = (*csv)[r_i];
                }
                while (r_i < r_count) {
                    if (b == EOF) {
                        break;
                    }
                    if (!quoted) {
                        if (b == '"') {
                            quoted = true;
                            continue;
                        } else if (b == '\r') {
                            continue;
                        }
                    }
                    if (b == ',' || b == '\n') {
                        f_i++;
                        // end of field
                        if (b == '\n') {
                            // end of record
                            r = (*csv)[++r_i];
                            f_i = 0;
                        }
                        // f = r[f_i];
                        continue;
                    }
                    r[f_i] += (char)b;
                    b = (*io).readByte();
                }
                // TODO:
                num_records_read = r_i;
            }
        public:
            std::shared_ptr<IOStream> io;
            size_t num_records_read;

            CSVParser(std::shared_ptr<IOStream> io, char delimeter): io(io), num_records_read(0) {}
            CSVParser(std::shared_ptr<IOStream> io): CSVParser(io, ',') {}
            ~CSVParser() = default;

            void readDoubles(CSV<double>& csv, size_t r_count) {
                uint8_t b = (*io).readByte();
                size_t r_i = 0;
                size_t f_i = 0;
                MatrixVector<double> r = csv[r_i];
                double current_double = 0;
                while (r_i < r_count) {
                    if (b == EOF) {
                        break;
                    }
                    if (b == ',' || b == '\n') {
                        r[f_i] = current_double;
                        current_double = 0;
                        f_i++;
                        // end of field
                        if (b == '\n') {
                            // end of record
                            r = csv[++r_i];
                            f_i = 0;
                        }
                        // f = r[f_i];
                        continue;
                    }
                    size_t dummy_digit_count = 0;
                    if (b == '.') {
                        (*io).readDecimal(current_double, dummy_digit_count);
                    } else {
                        (*io).readNatural(current_double, dummy_digit_count);
                    }
                    b = (*io).readByte();
                }
                // TODO:
                num_records_read = r_i;
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