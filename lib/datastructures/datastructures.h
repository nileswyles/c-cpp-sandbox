#ifndef WYLESLIBS_DATASTRUCTURES_H
#define WYLESLIBS_DATASTRUCTURES_H

#include <map>
#include <string>
#include <memory>

#ifndef ALGO_SIGNED_LONG
#define ALGO_SIGNED_LONG int64_t
#endif

#ifndef ALGO_UNSIGNED_LONG
#define ALGO_UNSIGNED_LONG uint64_t
#endif

#ifndef ALGO_SIGNED_INT
#define ALGO_SIGNED_INT int32_t
#endif

#ifndef ALGO_UNSIGNED_INT
#define ALGO_UNSIGNED_INT uint32_t
#endif

#include "datastructures/array.h"

namespace WylesLibs {

    class MatrixVectorView {
        public:
            size_t start;
            size_t end;
            MatrixVectorView(size_t start, size_t end): start(start), end(end) {}
            ~MatrixVectorView() = default;
    };

    // @

    template<typename T>
    class MatrixVector: public SharedArray<T> {
        private:
            // TODO: might move this to MatrixVector as originally considered lol... MAKE UP YOUR MIND!
            MatrixVectorView * view;
            size_t viewEnd() {
                if (this->view == nullptr) {
                    return this->size() - 1;
                } else {
                    // # inclusive...
                    return this->view->end;
                }
            }
            size_t viewStart() {
                if (this->view == nullptr) {
                    return 0;
                } else {
                    // # inclusive...
                    return this->view->start;
                }
            }
        public:
            MatrixVector() = default;
            MatrixVector(const size_t initial_cap): SharedArray<T>(initial_cap) {}
            // view
            MatrixVector(const MatrixVector<T>& other, size_t start, size_t end) {
                if (other.ctrl != nullptr) {
                    this->ctrl = other.ctrl;
                    // ! IMPORTANT 
                    size_t view_size = end - start;
                    if (view_size <= 0 || view_size > this->ctrl->ptr->size()) {
                        throw std::runtime_error("Invalid view coordinates.");
                    }
                    this->view = new MatrixVectorView(start, end);
                }
            }
            ~MatrixVector() {
                if (this->ctrl == nullptr || this->ctrl->instance_count == 1) {
                    delete this->view;
                }
                // ~SharedArray();
            };
            size_t size() {
                if (this->view == nullptr) {
                    return this->ctrl->ptr->size();
                } else {
                    // # inclusive...
                    return this->viewEnd() - this->viewStart() + 1;
                }
            }
            ALGO_UNSIGNED_INT euclidean(const MatrixVector<T>& v) {
                assertArraySizes(*this, v);
                size_t size = this->size();
                ALGO_SIGNED_INT sum = 0;
                for (size_t i = 0; i < size; i++) {
                    sum += ((ALGO_SIGNED_INT)(*this)[i] - v[i]) * ((ALGO_SIGNED_INT)(*this)[i] - v[i]);
                }
                return sqrt(sum);
            }
            ALGO_UNSIGNED_INT manhattan(const MatrixVector<T>& v) {
                assertArraySizes(*this, v);
                // # no C! lol, ef the pythagorean theorem.
                size_t size = this->size();
                ALGO_SIGNED_INT sum = 0;
                for (size_t i = 0; i < size; i++) {
                    sum += abs((ALGO_SIGNED_INT)(*this)[i] - v[i]);
                }
                return sqrt(sum);
            }
            // copy (not copy constructor)
            MatrixVector<T> copy(const MatrixVector<T>& other) {
                MatrixVector<T> copy(other.size());
                for (size_t i = other.viewStart(); i <= other.viewEnd(); i++) {
                    copy.append(other[i]);
                }
                return copy;
            }
            MatrixVector<T>& operator+ (const MatrixVector<T>& m) {
                assertArraySizes<T>(*this, m);
         
                size_t size = this->size();
                MatrixVector<T> add(size);
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + m[i];
                }
                return add;
            }
            MatrixVector<T>& operator- (const MatrixVector<T>& m) {
                assertArraySizes<T>(*this, m);
         
                size_t size = this->size();
                MatrixVector<T> sub(size);
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
         
                size_t size = this->size();
                T dot = 0;
                for (size_t i = 0; i < size; i++) {
                    dot += (*this)[i] * m[i];
                }
                return dot;
            }
            MatrixVector<T>& operator+ (const T& b) {
                size_t size = this->size();
                MatrixVector<T> add(size);
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + b;
                }
                return add;
            }
            MatrixVector<T>& operator- (const T& b) {
                size_t size = this->size();
                MatrixVector<T> sub(size);
                for (size_t i = 0; i < size; i++) {
                    sub[i] = (*this)[i] - b;
                }
                return sub;
            }
            MatrixVector<T>& operator* (const T& b) {
                size_t size = this->size();
                MatrixVector<T> dot(size);
                for (size_t i = 0; i < size; i++) {
                    dot += (*this)[i] * b;
                }
                return dot;
            }
            T& operator[] (const size_t pos) {
                size_t i = pos;
                if (this->view != nullptr) {
                    if (i > this->viewEnd()) {
                        std::runtime_error("Attempting to access element outside of MatrixVector.");
                    }
                    i += this->viewStart();
                    if (i >= this->size()) {
                        std::runtime_error("Attempting to access element outside of MatrixVector.");
                    }
                }
                return (*this->ctrl->ptr)[i];
            }
            // copy constructor - containerization code remains here
            MatrixVector(const MatrixVector<T>& other) {
                this->ctrl = other.ctrl;
                this->view = other.view;
                this->ctrl->instance_count++;
            }
            // copy assignment - containerization code remains here
            MatrixVector<T>& operator= (const MatrixVector<T>& other)  {
                this->ctrl = other.ctrl;
                this->view = other.view;
                this->ctrl->instance_count++;
                return *this;
            }
    };

    // @

    // TODO: vertical sort?
    //  Also, tensor is just Matrix<MatrixVector<T>>?
    template<typename T>
    class Matrix {
        protected:
            // TODO: yeah, so this means an extra 2-ptrs per row... (ctrl, view - in SharedArray) should be fine... flat not so useful in this situation lol
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
            MatrixVector<ALGO_UNSIGNED_INT> euclidean(const Matrix<T>& m) {
                assertMatrixSizes<T>(*this, m);
                size_t rows = this->rows();
                MatrixVector<T> m_out(this->columns());
                for (size_t i = 0; i < rows; i++) {
                    m_out[i] = (*this)[i].euclidean(m[i]);
                } 
                return m_out; 
            }
            MatrixVector<ALGO_UNSIGNED_INT> manhattan(const Matrix<T>& m) {
                assertMatrixSizes<T>(*this, m);
                size_t rows = this->rows();
                MatrixVector<T> m_out(this->columns());
                for (size_t i = 0; i < rows; i++) {
                    m_out[i] = (*this)[i].manhattan(m[i]);
                } 
                return m_out;
            }
            // matrix copy constructor but not actual copy constructor lol... 
            Matrix<T> copy(const Matrix<T>& other) {
                Matrix<T> copy;
                MatrixVector<MatrixVector<T>> y_vector(other.matrix.size());
                // # inclusive
                for (size_t i = *other.matrix.viewStart(); i <= other.matrix.viewEnd(); i++) {
                    y_vector[i] = other.matrix[i].copy();
                }
                copy.matrix = y_vector;
                return copy;
            }
            Matrix<T> view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
                size_t x_size = x_end - x_start;
                size_t y_size = y_end - y_start;
                if (y_start >= this->rows() || y_start < 0 ||
                        x_start >= this->columns() || x_start < 0 ||
                        x_size == 0 || x_size > this->columns() || 
                        y_size == 0 || y_size > this->rows()) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                Matrix<T> view;
                MatrixVector<MatrixVector<T>> y_vector(this->matrix, y_start, y_end);
                // # inclusive
                for (size_t i = this->matrix.viewStart(); i <= this->matrix.viewEnd(); i++) {
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
            SharedArray<std::shared_ptr<GraphNode>> node_list;
            Graph() = default;
            virtual ~Graph() = default;
    };

    // @

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