#ifndef WYLESLIBS_DATASTRUCTURES_H
#define WYLESLIBS_DATASTRUCTURES_H

#include "datastructures/array.h"
#include "memory/pointers.h"

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

namespace WylesLibs {

    // @
    class MatrixVectorView {
        public:
            // ptr?
            // template<typename T>
            // friend MatrixVector<T> vector; // lol... because we're trying to include examples of all language features? but in reality this is RETARDED
            // MatrixVectorView(MatrixVector<T> vector, size_t start, size_t end): vector(vector), start(start), end(end) {}
            size_t start;
            size_t end;
            MatrixVectorView(size_t start, size_t end): start(start), end(end) {}
            ~MatrixVectorView() = default;
            bool operator== (const MatrixVectorView& other) {
                return this->start == other.start && this->end == other.end;
            }
    };

    // @

    template<typename T>
    class MatrixVector: public SharedArray<T> {
        private:
            // TODO: might move this to MatrixVector as originally considered lol... MAKE UP YOUR MIND!
            MatrixVectorView * view_obj;

            static void assertArraySizes(const MatrixVector<T>& v1, const MatrixVector<T>& v2) {
                if (v1.size() != v2.size()) {
                    printf("%lu, %lu\n", v1.size(), v2.size());
                    throw std::runtime_error("Vector sizes should equal.");
                }
            }
        public:
            MatrixVector() = default;
            MatrixVector(std::initializer_list<T> list): SharedArray<T>(list) {}
            MatrixVector(std::string s): SharedArray<T>(s) {}
            MatrixVector(SharedArray<T> a): SharedArray<T>(a) {}
            MatrixVector(const size_t initial_cap): MatrixVector<T>(initial_cap, false) {}
            MatrixVector(const size_t initial_cap, bool is_view): SharedArray<T>(initial_cap) {
                // TODO: what in the world? this needs unit testing but I'm not sure this was the intended functionality here.
                if (initial_cap <= 1) {
                    throw std::runtime_error("View must have at least two elements (remember inclusive).");
                }
                this->view_obj = new MatrixVectorView(0, initial_cap - 1);
            }
            // view
            MatrixVector(const MatrixVector<T>& other, size_t start, size_t end) {
                if (other.ctrl != nullptr) {
                    this->ctrl = other.ctrl;
                    // ! IMPORTANT 
                    size_t view_size = end - start;
                    if (view_size <= 0 || view_size > this->ctrl->ptr->size()) {
                        throw std::runtime_error("Invalid view coordinates.");
                    }
                    this->view_obj = new MatrixVectorView(start, end);
                }
            }
            ~MatrixVector() {
                if (this->ctrl == nullptr || this->ctrl->instance_count == 1) {
                    delete this->view_obj;
                }
                // ~SharedArray();
            };
            void view(MatrixVector<T> * cont, size_t start, size_t end) {
                *cont = MatrixVector<T>(this, start, end);
            }
            size_t viewEnd() {
                if (this->view_obj == nullptr) {
                    // TODO: must have at least one element?
                    return this->size() - 1;
                } else {
                    // # inclusive...
                    return this->view_obj->end;
                }
            }
            size_t viewStart() {
                if (this->view_obj == nullptr) {
                    return 0;
                } else {
                    // # inclusive...
                    return this->view_obj->start;
                }
            }
            // TODO: lol, didn't implement the begin/end functionality here.
            MatrixVector<T>& insert(const size_t pos, const T * els, const size_t num_els) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->insert(pos, els, num_els);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& insert(const size_t pos, const T& el) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->insert(pos, el);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& uniqueAppend(const T& el) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->uniqueAppend(el);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& append(const T& el) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->append(el);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& append(const MatrixVector<T>& other) {
                if (this->view_obj == nullptr) {
                    return this->append(other.begin(), other.size());
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
            }
            MatrixVector<T>& append(const T * els, const size_t num_els) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->append(els, num_els);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& remove(const size_t pos, const size_t num_els) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->remove(pos, num_els);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& removeEl(const T& el) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->removeEl(el);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& remove(const size_t pos) override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->remove(pos);
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& removeFront() override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->removeFront();
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            MatrixVector<T>& removeBack() override {
                if (this->view_obj == nullptr) {
                    this->ctrl->ptr->removeBack();
                } else {
                    std::string message = "Cannot modify a view-only matrix vector.";
                    loggerPrintf(LOGGER_DEBUG_VERBOSE, "%s\n", message.c_str());
                    throw std::runtime_error(message);
                }
                return *this;
            }
            size_t size() override {
                if (this->view_obj == nullptr) {
                    return this->ctrl->ptr->size();
                } else {
                    // # inclusive...
                    return this->viewEnd() - this->viewStart() + 1;
                }
            }
            ALGO_UNSIGNED_INT euclidean(const MatrixVector<T>& v) {
                MatrixVector::assertArraySizes(*this, v);
                size_t size = this->size();
                ALGO_SIGNED_INT sum = 0;
                for (size_t i = 0; i < size; i++) {
                    sum += ((ALGO_SIGNED_INT)(*this)[i] - v[i]) * ((ALGO_SIGNED_INT)(*this)[i] - v[i]);
                }
                return sqrt(sum);
            }
            ALGO_UNSIGNED_INT manhattan(const MatrixVector<T>& v) {
                MatrixVector::assertArraySizes(*this, v);
                // # no C! lol, ef the pythagorean theorem.
                size_t size = this->size();
                ALGO_SIGNED_INT sum = 0;
                for (size_t i = 0; i < size; i++) {
                    sum += abs((ALGO_SIGNED_INT)(*this)[i] - v[i]);
                }
                return sqrt(sum);
            }
            // copy (not copy constructor)
            MatrixVector<T> copy(size_t initial_cap = SIZE_MAX) {
                if (initial_cap == SIZE_MAX) {
                    initial_cap = this->cap();
                }
                MatrixVector<T> c(initial_cap < this->size() ? this->cap(): initial_cap);
                for (size_t i = this->viewStart(); i <= this->viewEnd(); i++) {
                    c.append((*this)[i]);
                }
                return c;
            }
            MatrixVector<T>& operator+ (const MatrixVector<T>& m) {
                MatrixVector::assertArraySizes(*this, m);
         
                size_t size = this->size();
                MatrixVector<T> add(size);
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + m[i];
                }
                return add;
            }
            MatrixVector<T>& operator- (const MatrixVector<T>& m) {
                MatrixVector::assertArraySizes(*this, m);
         
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
                MatrixVector::assertArraySizes(*this, m);
         
                size_t size = this->size();
                T dot = 0;
                for (size_t i = 0; i < size; i++) {
                    dot += (*this)[i] * m[i];
                }
                return dot;
            }

#if defined(MATRIX_VECTOR_SCALAR_ADDITION)
            MatrixVector<T>& operator+ (const T& b) {
                size_t size = this->size();
                MatrixVector<T> add(size);
                for (size_t i = 0; i < size; i++) {
                    add[i] = (*this)[i] + b;
                }
                return add;
            }
#endif
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
            // TODO:
            //      this should be const, does const reference have some other semantic?
            bool operator== (MatrixVector<T>& other) {
                bool equals = true;
                size_t size = this->size();
                if (other.size() != this->size()) {
                    equals = false;
                } else {
                    for (size_t i = 0; i < size; i++) {
                        if ((*this)[i] != other[i]) {
                            equals = false;
                            break;
                        }
                    }
                }
                return equals && this->view_obj == other.view_obj;
            }
            T& operator[] (const size_t pos) {
                size_t i = pos;
                if (this->view_obj != nullptr) {
                    i += this->viewStart();
                    if (i > this->viewEnd()) {
                        std::runtime_error("Attempting to access element outside of MatrixVector.");
                    } else if (i <= this->ctrl->ptr->size()) {
                        //  Let's keep it simple - can't edit views. If main MatrixVector is modified, the view might lie outside of original MatrixVector, so throw exception.
                        std::runtime_error("Attempting to access element outside of underlying buffer.");
                    }
                }
                return (*this->ctrl->ptr)[i];
            }

#if !defined(MATRIX_VECTOR_SCALAR_ADDITION)
            MatrixVector<T>& operator+ (T& x) {
                this->append(x);
                return *this;
            }
#endif
            // copy constructor - containerization code remains here
            MatrixVector(const MatrixVector<T>& other) {
                this->ctrl = other.ctrl;
                this->view_obj = other.view_obj;
                this->ctrl->instance_count++;
            }
            // copy assignment - containerization code remains here
            MatrixVector<T>& operator= (const MatrixVector<T>& other)  {
                this->ctrl = other.ctrl;
                this->view_obj = other.view_obj;
                this->ctrl->instance_count++;
                return *this;
            }
            // Move constructor
            MatrixVector(MatrixVector<T>&& x) {
                std::swap(this->ctrl, x.ctrl);
                std::swap(this->view_obj, x.view_obj);
            }
            // Move assignment
            MatrixVector<T>& operator= (MatrixVector<T>&& x) {
                std::swap(this->ctrl, x.ctrl);
                std::swap(this->view_obj, x.view_obj);
                return *this;
            }
    };

    // @

    // TODO: vertical sort?
    //  Also, tensor is just Matrix<MatrixVector<T>>?
    template<typename T>
    class Matrix {
        private:
            // TODO: lol, this is interesting...
            static void assertMatrixSizes(const Matrix<T>& m1, const Matrix<T>& m2) {
                if (m1.rows() != m2.rows() || m1.columns() != m2.columns()) {
                    throw std::runtime_error("Invalid shape of matricies.");
                }
            }
            static void assertTMatrixSizes(const Matrix<T>& m1, const Matrix<T>& m2) {
                if (m1.rows() != m2.columns() || m1.columns() != m2.rows()) {
                    throw std::runtime_error("Invalid shape of matricies.");
                }
            }
        protected:
            // TODO: yeah, so this means an extra 2-ptrs per row... (ctrl, view - in MatrixVector) should be fine... flat not so useful in this situation lol
            MatrixVector<MatrixVector<T>> matrix;
        public:
            Matrix() = default;
            Matrix(std::initializer_list<MatrixVector<T>> list) {
                matrix = MatrixVector<MatrixVector<T>>(list.size(), true);
                for (auto el: list) {
                    matrix.append(el);
                }
            }
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
                Matrix::assertMatrixSizes(*this, m);
                size_t rows = this->rows();
                MatrixVector<T> m_out(this->columns());
                for (size_t i = 0; i < rows; i++) {
                    m_out[i] = (*this)[i].euclidean(m[i]);
                } 
                return m_out; 
            }
            MatrixVector<ALGO_UNSIGNED_INT> manhattan(const Matrix<T>& m) {
                Matrix::assertMatrixSizes(*this, m);
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
                MatrixVector<MatrixVector<T>> y_vector(other.matrix.cap());
                // # inclusive
                for (size_t i = *other.matrix.viewStart(); i <= other.matrix.viewEnd(); i++) {
                    y_vector[i] = other.matrix[i].copy();
                }
                copy.matrix = y_vector;
                return copy;
            }
            Matrix<T> view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
                int64_t x_size = (int64_t)x_end - x_start;
                int64_t y_size = (int64_t)y_end - y_start;
                // ! IMPORTANT 
                //      -Wsigncompare 
                //      Solution:
                //          cast from size_t to int64_t. Overflow very unlikely.
                if (y_start >= this->rows() ||
                        x_start >= this->columns() ||
                        x_size == 0 || x_size > (int64_t)this->columns() || 
                        y_size == 0 || y_size > (int64_t)this->rows()) {
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
                Matrix::assertMatrixSizes(*this, m);
                Matrix<T> add;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        add[x][y] = (*this)[x][y] + m[x][y];
                    }
                }
                return add;
            }
            Matrix<T>& operator- (const Matrix<T>& m) {
                Matrix::assertMatrixSizes(*this, m);
                Matrix<T> sub;
                for (size_t x = 0; x < this->rows(); x++) {
                    for (size_t y = 0; y < this->columns(); y++) {
                        sub[x][y] = (*this)[x][y] - m[x][y];
                    }
                }
                return sub;
            }
            Matrix<T>& operator* (const Matrix<T>& m) {
                Matrix::assertTMatrixSizes(*this, m);
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
            bool operator== (const Matrix<T>& other) {
                bool equals = true;
                size_t rows = this->rows();
                if (other.rows() != this->rows() || other.columns() != this->columns()) {
                    equals = false;
                } else {
                    for (size_t i = 0; i < rows; i++) {
                        if (false == this->matrix[i] == other->matrix[i]) {
                            equals = false;
                            break;
                        }
                    }
                }
                return equals 
                    && this->matrix.viewEnd() == other.matrix.viewEnd() 
                    && this->matrix.viewStart() == other.matrix.viewStart();
            }
    };
    class GraphNode {
        public:
            std::map<std::string, ESharedPtr<GraphNode>> adjacency_map;
            GraphNode() = default;
            virtual ~GraphNode() = default;
    };
    class Graph {
        public:
            // maybe i don't need shared ptr because I can keep this contained but let's seee...
            SharedArray<ESharedPtr<GraphNode>> node_list;
            Graph() = default;
            virtual ~Graph() = default;
    };

};
#endif
