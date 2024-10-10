#ifndef WYLESLIBS_CSV_H
#define WYLESLIBS_CSV_H

#include "iostream/iostream.h"
#include "datastructures/datastructures.h"
#include <string>
#include <memory>

using namespace WylesLibs::DS;

namespace WylesLibs {
    template<typename T>
    class CSV: public Matrix {
        public:
            MatrixVector<std::string> header;
            CSV() = default;
            ~CSV() = default;
    };
    class CSVParser {
        public:
            bool no_header;
            std::shared_ptr<IOStream> io;
            size_t num_records_read;
            // TODO:
            // hmm... yeah consider whether the header flags are even needed? can leave that up to whatever is calling this?
            CSVParser(std::shared_ptr<IOStream> io, char delimeter, bool no_header): io(io), no_header(no_header), num_records_read(0) {}
            CSVParser(std::shared_ptr<IOStream> io, char delimeter): CSVParser(io, delimeter, false) {}
            CSVParser(std::shared_ptr<IOStream> io): CSVParser(io, ',', false) {}
            ~CSVParser() = default;

            CSV<double> readDoubles() {
                return readDoubles(SIZE_MAX);
            }
            CSV<double> readDoubles(size_t r_count) {
                CSV<double> csv;
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
                return csv;
            }
            // TODO: yeah, maybe not even bother with this
            // CSV<int64_t> readInt64s() {}
            // CSV<int64_t> readInt64s(size_t r_count) {
            //     CSV<int64_t> csv;
            //     uint8_t b = (*io).readByte();
            //     size_t r_i = 0;
            //     size_t f_i = 0;
            //     MatrixVector<int64_t> r = csv[r_i];
            //     double current_double = 0;
            //     while (r_i < r_count) {
            //         if (b == EOF) {
            //             break;
            //         }
            //         if (b == ',' || b == '\n') {
            //             r[f_i] = (int64_t)current_double;
            //             current_double = 0;
            //             f_i++;
            //             // end of field
            //             if (b == '\n') {
            //                 // end of record
            //                 r = csv[++r_i];
            //                 f_i = 0;
            //             }
            //             // f = r[f_i];
            //             continue;
            //         }
            //         size_t dummy_digit_count = 0;
            //         (*io).readNatural(current_double, dummy_digit_count);
            //         b = (*io).readByte();
            //     }
            //     return csv;
                // // TODO:
                // num_records_read = r_i;
            // }

            // CSV<int32_t> readInt32s() {}
            // CSV<int32_t> readInt32s(size_t r_count) {
            //     CSV<int32_t> csv;
            //     uint8_t b = (*io).readByte();
            //     size_t r_i = 0;
            //     size_t f_i = 0;
            //     MatrixVector<int32_t> r = csv[r_i];
            //     double current_double = 0;
            //     while (r_i < r_count) {
            //         if (b == EOF) {
            //             break;
            //         }
            //         if (b == ',' || b == '\n') {
            //             r[f_i] = (int64_t)current_double;
            //             current_double = 0;
            //             f_i++;
            //             // end of field
            //             if (b == '\n') {
            //                 // end of record
            //                 r = csv[++r_i];
            //                 f_i = 0;
            //             }
            //             // f = r[f_i];
            //             continue;
            //         }
            //         size_t dummy_digit_count = 0;
            //         (*io).readNatural(r[f_i], dummy_digit_count);
            //         b = (*io).readByte();
            //     }
                // // TODO:
                // num_records_read = r_i;
            //     return csv;
            // }

            CSV<std::string> read() {
                return read(SIZE_MAX);
            }
            CSV<std::string> read(size_t r_count) {
                CSV<std::string> csv;
                bool quoted = false;
                uint8_t b = (*io).readByte();
                size_t r_i = 0;
                size_t f_i = 0;
                MatrixVector<std::string> r = csv[r_i];
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
                            r = csv[++r_i];
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
                return csv;
            }
    };
};

#endif 