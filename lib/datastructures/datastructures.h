#ifndef WYLESLIBS_DATASTRUCTURES_H
#define WYLESLIBS_DATASTRUCTURES_H

#include <map>
#include <string>
#include <memory>

#include "datastructures/array.h"

namespace WylesLibs {
    template<typename T>
    class MatrixVector: public Array<T> {
        private:
            size_t * e_start;
            size_t * e_end;
        public:
            MatrixVector(): Array<T>(), e_start(new size_t(0)), e_end(new size_t(0)) {}
            MatrixVector(const MatrixVector<T>& other, size_t start, size_t end): Array<T>((Array<T> *)&other) {
                // view...
                (*other.instance_count)++;

                size_t view_size = end - start;
                if (view_size == 0 || view_size > *other.e_size) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                e_start = new size_t(start); 
                e_end = new size_t(end); 
            }
            ~MatrixVector() override {
                if (*this->instance_count == 1) {
                    if (this->e_start != nullptr) {
                        delete this->e_start;
                    } 
                    if (this->e_end != nullptr) {
                        delete this->e_end;
                    }
                }
                // ~Array() is called...
            }
            size_t size() {
                size_t size = *this->e_end - *this->e_start;
                if (false == size > 0) {
                    return size;
                } else {
                    return *this->e_size;
                }
            }
            // not to be confused with copy constructor
            MatrixVector<T> copy(const MatrixVector<T>& other) {
                MatrixVector<T> copy;
                size_t size = copy.size();
                for (size_t i = *other.e_start; i < size; i++) {
                    copy.append(other.buf()[i]);
                }
                return copy;
            }
            MatrixVector<T>& operator+ (const MatrixVector<T>& m) {
                assertArraySizes<T>(*this, m);
         
                size_t size = this->size();
                MatrixVector<T> add;
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + m[i];
                }
                return add;
            }
            MatrixVector<T>& operator- (const MatrixVector<T>& m) {
                assertArraySizes<T>(*this, m);
         
                size_t size = this->size();
                MatrixVector<T> sub;
                for (size_t i = 0; i < size; i++) {
                    sub[i] = (*this)[i] - m[i];
                }
                return sub;
            }
            T& operator* (const MatrixVector<T>& m) {
                // # a dot b = |a||b|cos(theta) 
                //  TODO: look up proof of that ^
                //  applications:  
                //      law of cosines
                assertArraySizes<T>(*this, m);
         
                size_t a_size = this->size();
                T dot = 0;
                for (size_t i = 0; i < a_size; i++) {
                    dot += (*this)[i] * m[i];
                }
                return dot;
            }
            MatrixVector<T>& operator+ (const T& b) {
                size_t size = this->size();
                MatrixVector<T> add;
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + b;
                }
                return add;
            }
            MatrixVector<T>& operator- (const T& b) {
                size_t size = this->size();
                MatrixVector<T> sub;
                for (size_t i = 0; i < size; i++) {
                    sub[i] = (*this)[i] - b;
                }
                return sub;
            }
            MatrixVector<T>& operator* (const T& b) {
                // lol, is this even a nece?
                size_t a_size = this->size();
                MatrixVector<T> dot;
                for (size_t i = 0; i < a_size; i++) {
                    dot += (*this)[i] * b;
                }
                return dot;
            }
            T& operator[] (const size_t pos) {
                size_t i = pos;
                if (true == (*this->e_end - *this->e_start) > 0) {
                    if (i > *this->e_end) {
                        std::runtime_error("Attempting to access element outside of Matrix.");
                    }
                    i += *this->e_start;
                    if (i >= this->size()) {
                        std::runtime_error("Attempting to access element outside of Matrix.");
                    }
                } else {
                    // TODO: maybe don't even bother with this...
                    T el;
                    this->append(el); 
                    i = this->size()-1;
                }
                return (*this->e_buf)[i];
            }
            // copy constructor - containerization code remains here
            MatrixVector(const MatrixVector<T>& other): Array<T>((Array<T> *)&other) {
                this->e_start = other.e_start;
                this->e_end = other.e_end;
            }
            // copy assignment - containerization code remains here
            MatrixVector<T>& operator= (const MatrixVector<T>& other)  {
                if (other.instance_count == nullptr || other.e_cap == nullptr || other.e_size == nullptr || other.e_sorted == nullptr) {
                    std::runtime_error("Make sure the array is initialized before trying to create");
                } else {
                    this->instance_count = other.instance_count;
                    this->e_buf = other.e_buf;
                    this->e_cap = other.e_cap;
                    this->e_size = other.e_size;
                    this->e_sorted = other.e_sorted;
                    this->e_start = other.e_start;
                    this->e_end = other.e_end;
                    (*this->instance_count)++;
                }
                return *this;
            }
    };
    template<typename T>
    class Matrix {
        protected:
            MatrixVector<MatrixVector<T>> matrix;
        public:
            Matrix() = default;
            virtual ~Matrix() = default;
            // ! IMPORTANT - let's be clear, the user has control and needs to ensure matrix is structured properly when populating...
            size_t rows() {
                return matrix.size();
            }
            size_t columns() {
                if (this->rows() == 0) {
                    return 0;
                }
                return (*this)[0].size();
            }
            // matrix copy constructor but not actual copy constructor lol... 
            Matrix<T> copy(const Matrix<T>& other) {
                Matrix<T> copy;
                MatrixVector<MatrixVector<T>> y_vector;
                size_t end = other.size();
                if (*other.matrix.e_end != 0) {
                    end = *other.matrix.e_end;
                }
                for (size_t i = *other.matrix.e_start; i < end; i++) {
                    y_vector[i] = other.matrix[i].copy();
                }
                copy.matrix = y_vector;
                return copy;
            }
            Matrix<T> view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
                size_t x_size = x_end - x_start;
                size_t y_size = y_end - y_start;
                // TODO: think about getting view of view...
                //  view of copy...
                //  TDD lol..
                if (y_start >= this->rows() || y_start < 0 ||
                        x_start >= this->columns() || x_start < 0 ||
                        x_size == 0 || x_size > this->columns() || 
                        y_size == 0 || y_size > this->rows()) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                Matrix<T> view;
                MatrixVector<MatrixVector<T>> y_vector(this->matrix, y_start, y_end);

                size_t end = this->rows();
                if (*this->matrix.e_end != 0) {
                    end = *this->matrix.e_end;
                }
                for (size_t i = *this->matrix.e_start; i < end; i++) {
                    MatrixVector<T> x_vector(this->matrix[i], x_start, x_end);
                    y_vector[i] = x_vector;
                }
                view.matrix = y_vector;
                return view;
            }
            Matrix<T>& operator+ (const Matrix<T>& m) {
                assertMatrixSizes<T>(*this, m);
                Matrix<T> add;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        add[x][y] = (*this)[x][y] + m[x][y];
                    }
                }
                return add;
            }
            Matrix<T>& operator- (const Matrix<T>& m) {
                assertMatrixSizes<T>(*this, m);
                Matrix<T> sub;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        sub[x][y] = (*this)[x][y] - m[x][y];
                    }
                }
                return sub;
            }
            Matrix<T>& operator* (const Matrix<T>& m) {
                assertTMatrixSizes<T>(*this, m);
                Matrix<T> mul;
                return mul;
            }
            Matrix<T>& operator+ (const T& b) {
                Matrix<T> add;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        add[x][y] = (*this)[x][y] + b;
                    }
                }
                return add;
            }
            Matrix<T>& operator- (const T& b) {
                Matrix<T> sub;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        sub[x][y] = (*this)[x][y] - b;
                    }
                }
                return sub;
            }
            Matrix<T>& operator* (const T& b) {
                Matrix<T> mul;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        mul[x][y] = (*this)[x][y] * b;
                    }
                }
                return mul;
            }
            MatrixVector<T>& operator[] (const size_t pos) {
                return this->matrix[pos];
            }
    };
    class GraphNode {
        public:
            std::map<std::string, std::shared_ptr<GraphNode>> adjacency_map;
            GraphNode() = default;
            virtual ~GraphNode() = default;
    };
    class Graph {
        public:
            // maybe i don't need shared ptr because I can keep this contained but let's seee...
            Array<std::shared_ptr<GraphNode>> node_list;
            Graph() = default;
            virtual ~Graph() = default;
    };

    template<typename T>
    static void assertArraySizes(const MatrixVector<T>& v1, const MatrixVector<T>& v2) {
        if (v1.size() != v2.size()) {
            printf("%lu, %lu\n", v1.size(), v2.size());
            throw std::runtime_error("Vector sizes should equal.");
        }
    }
    template<typename T>
    static void assertMatrixSizes(const Matrix<T>& m1, const Matrix<T>& m2) {
        if (m1.rows() != m2.rows() || m1.columns() != m2.columns()) {
            throw std::runtime_error("Invalid shape of matricies.");
        }
    }
    template<typename T>
    static void assertTMatrixSizes(const Matrix<T>& m1, const Matrix<T>& m2) {
        if (m1.rows() != m2.columns() || m1.columns() != m2.rows()) {
            throw std::runtime_error("Invalid shape of matricies.");
        }
    }
};
#endif