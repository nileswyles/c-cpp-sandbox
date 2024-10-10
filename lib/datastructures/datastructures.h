#ifndef WYLESLIBS_DATASTRUCTURES_H
#define WYLESLIBS_DATASTRUCTURES_H

#include <map>
#include <string>
#include <memory>

#include "datastructures/array.h"

using namespace WylesLibs::Algo;

namespace WylesLibs::DS {
    template<typename T>
    class MatrixVector: public Array {
        private:
            size_t * e_start;
            size_t * e_end;

            MatrixVector(): e_start(new size_t(0)), e_end(new size_t(0)) {}
            MatrixVector(const MatrixVector<T>& other, size_t start, size_t end) {
                // view...
                instance_count = other.instance_count;
                e_buf = other.e_buf;
                e_cap = other.e_cap;
                e_size = other.e_size;
                e_sorted = other.e_sorted;
                (*this->instance_count)++;

                size_t view_size = end - start;
                if (view_size == 0 || view_size > *e_size) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                e_start = new size_t(start); 
                e_end = new size_t(end); 
            }
            ~MatrixVector() override final {
                // TODO: order of calling destructors? first time inheriteded container type...
                // (*this->instance_count)--;

                // assuming parent destructor is called automatically after this? let's check if instance = 1...
                if (*this->instance_count == 1) {
                    if (e_start != nullptr) {
                        delete e_start;
                    } 
                    if (e_end != nullptr) {
                        delete e_end;
                    }
                }
            }
            // because not same as copy constructor
            MatrixVector<T> copy(const MatrixVector<T>& other) {
                MatrixVector<T> copy;
                // TODO: is_view seems not necessary now but maybe it is - runtime_error?
                size_t size = *other.e_end - *other.e_start;
                size_t loop_start = *other.e_start;
                if (false == size > 0) {
                    loop_start = 0; 
                    size = other.size();
                }
                for (size_t i = loop_start; i < size; i++) {
                    copy.append(other.buf()[i]);
                }
                return copy;
            }
            MatrixVector<T>& operator[] (const size_t pos) {
                size_t i = pos;
                if (true == (*this->e_end - *this->e_start) > 0) {
                    if (i > this->*e_end) {
                        std::runtime_error("Attempting to access element outside of Matrix.");
                    }
                    i += *this-e_start;
                    if (i >= this->size()) {
                        std::runtime_error("Attempting to access element outside of Matrix.");
                    }
                } else {
                    T el;
                    this->append(el); 
                    i = this->size()-1;
                }
                return (*this->e_buf)[i];
            }
    };
    template<typename T>
    class Matrix {
        private:
            size_t * instance_count;
            size_t * x_start;
            size_t * x_end;
            size_t * y_start;
            size_t * y_end;
        public:
            MatrixVector<MatrixVector<T>> matrix;
            Matrix(size_t * instance_count, size_t x_start, size_t x_end, size_t y_start, size_t y_end):  {
                if (instance_count == nullptr) {
                    instance_count = new size_t(1);
                } else {
                    instance_count = instance_count;
                    (*instance_count)++;
                }
                x_start = new size_t(x_start);
                x_end = new size_t(x_end);
                y_start = new size_t(y_start);
                y_end = new size_t(y_end);
            }
            Matrix(): Matrix(nullptr, 0, 0, 0, 0) {}
            virtual ~Matrix() {
                (*this->instance_count)--;
                if (*this->instance_count == 0) {
                    if (instance_count != nullptr) {
                        delete instance_count;
                    }
                    if (x_start != nullptr) {
                        delete x_start;
                    } 
                    if (x_end != nullptr) {
                        delete x_end;
                    }
                    if (y_start != nullptr) {
                        delete y_start;
                    } 
                    if (y_end != nullptr) {
                        delete y_end;
                    }
                }
            }
            size_t rows() {
                if (*y_end - *y_start > 0) {
                    return *y_end - *y_start;
                } else {
                    return matrix.size();
                }
            }
            size_t columns() {
                if (*x_end - *x_start > 0) {
                    return *x_end - *x_start;
                } else {
                    // TODO: how to set limit/strict requirements? Do I care? Should I make immutable? unremovable at least?
                    // TODO:
                    // the above implies there's some data there?... that said, underlying arrays aren't immutable?
                    if (this->rows() == 0) {
                        return 0;
                    }
                    return (*this)[0].size();
                }
            }
            Matrix<T>& operator+ (const Matrix<T>& m) {
                return Space::plus(*this, m);
            }
            Matrix<T>& operator- (const Matrix<T>& m) {
                return Space::sub(*this, m);
            }
            Matrix<T>& operator* (const Matrix<T>& m) {
                return Space::mul(*this, m);
            }
            // matrix copy constructor but not actual copy constructor lol... 
            Matrix<T> copy(const Matrix<T>& other) {
                Matrix<T> copy;
                MatrixVector<MatrixVector<T>> y_vector;
                size_t loop_start = *y_start;
                if (*y_end - *y_start > 0) {
                    loop_start = 0; 
                }
                for (size_t i = loop_start; i < this->rows(); i++) {
                    y_vector[i] = other.matrix[i].copy();
                }
                copy.matrix = y_vector;
                return copy;
            }
            Matrix<T> view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
                size_t x_size = x_end - x_start;
                size_t y_size = y_end - y_start;
                if (x_size == 0 || x_size > this->columns() || y_size == 0 || y_size > this->rows()) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                Matrix<T> view(*instance_count, x_start, x_end, y_start, y_end);
                // TODO:
                // seems tedious/clunky
                MatrixVector<MatrixVector<T>> y_vector(this->matrix, y_start, y_end);
                for (size_t i = 0; i < this->rows(); i++) {
                    MatrixVector<T> x_vector(this->matrix[i], x_start, x_end);
                    y_vector[i] = x_vector;
                }
                view.matrix = y_vector;
                return view;
            }
            MatrixVector<T>& operator[] (const size_t pos) {
                return matrix[pos];
            }
            // Copy
            Matrix(const Matrix<T>& x) {
                this->instance_count = x.instance_count;
                this->y_start = x.start;
                this->y_end = x.end;
                this->x_start = x.start;
                this->x_end = x.end;
     
                (*this->instance_count)++;
            }
            Matrix<T>& operator= (const Matrix<T>& x) {
                this->instance_count = x.instance_count;
                this->y_start = x.start;
                this->y_end = x.end;
                this->x_start = x.start;
                this->x_end = x.end;
     
                (*this->instance_count)++;
                return *this;
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
};
#endif