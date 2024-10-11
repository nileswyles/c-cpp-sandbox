#include "datastructures/datastructures.h"
#include "algo/algo.h"

using namespace WylesLibs;

template<typename T>
MatrixVector<T> MatrixVector<T>::copy(const MatrixVector<T>& other) {
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
template<typename T>
MatrixVector<T>& MatrixVector<T>::operator+ (const MatrixVector<T>& m) {
    return Space::add<T>(*this, m);
}
template<typename T>
MatrixVector<T>& MatrixVector<T>::operator- (const MatrixVector<T>& m) {
    return Space::sub<T>(*this, m);
}
template<typename T>
MatrixVector<T>& MatrixVector<T>::operator* (const MatrixVector<T>& m) {
    return Space::mul<T>(*this, m);
}
template<typename T>
T& MatrixVector<T>::operator[] (const size_t pos) {
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

template<typename T>
size_t Matrix<T>::rows() {
    if (*e_y_end - *e_y_start > 0) {
        return *e_y_end - *e_y_start;
    } else {
        return matrix.size();
    }
}
template<typename T>
size_t Matrix<T>::columns() {
    if (*e_x_end - *e_x_start > 0) {
        return *e_x_end - *e_x_start;
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
// matrix copy constructor but not actual copy constructor lol... 
template<typename T>
Matrix<T> Matrix<T>::copy(const Matrix<T>& other) {
    Matrix<T> copy;
    MatrixVector<MatrixVector<T>> y_vector;
    size_t loop_start = *e_y_start;
    if (*e_y_end - *e_y_start > 0) {
        loop_start = 0; 
    }
    for (size_t i = loop_start; i < this->rows(); i++) {
        y_vector[i] = other.matrix[i].copy();
    }
    copy.matrix = y_vector;
    return copy;
}
template<typename T>
Matrix<T> Matrix<T>::view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
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
template<typename T>
Matrix<T>& Matrix<T>::operator+ (const Matrix<T>& m) {
    return Space::add<T>(*this, m);
}
template<typename T>
Matrix<T>& Matrix<T>::operator- (const Matrix<T>& m) {
    return Space::sub<T>(*this, m);
}
template<typename T>
Matrix<T>& Matrix<T>::operator* (const Matrix<T>& m) {
    return Space::mul<T>(*this, m);
}
template<typename T>
MatrixVector<T>& Matrix<T>::operator[] (const size_t pos) {
    return this->matrix[pos];
}