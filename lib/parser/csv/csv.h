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
                // TODO: this is dumb, why allocate an extra string?
                std::string current_str;
                while (r_i < r_count) {
                    if (b >= 0x20 && b <= 0x7E) {
                        printf("%c", b);
                    } else if (b == 0x0A) {
                        printf("\n");
                    } else {
                        printf("%x", b);
                    }
                    if (b == (uint8_t)EOF) {
                        printf("okay, so this isn't a thing?\n");
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
                        r.append(current_str);
                        current_str = "";
                        f_i++;
                        // end of field
                        if (b == '\n') {
                            // end of record
                            // #containerization
                            r = (*csv)[++r_i];
                            f_i = 0;
                        }
                    }
                    current_str.push_back((char)b);
                    b = (*io).readByte();
                }
                num_records_read = r_i + 1;
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
                    if (b == (uint8_t)EOF) {
                        break;
                    }
                    if (b == ',' || b == '\n') {
                        r[f_i] = current_double;
                        current_double = 0;
                        f_i++;
                        // end of field
                        if (b == '\n') {
                            // end of record
                            // #containerization
                            r = csv[++r_i];
                            f_i = 0;
                        }
                    }
                    size_t dummy_digit_count = 0;
                    if (b == '.') {
                        (*io).readDecimal(current_double, dummy_digit_count);
                    } else {
                        (*io).readNatural(current_double, dummy_digit_count);
                    }
                    b = (*io).readByte();
                }
                num_records_read = r_i + 1;
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