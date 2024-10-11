#include "datastructures/datastructures.h"
#include "algo/algo.h"

using namespace WylesLibs;

template<typename T>
size_t MatrixVector<T>::size() {
    size_t size = *this.e_end - *this.e_start;
    if (false == size > 0) {
        return size;
    } else {
        return *this->e_size;
    }
}
template<typename T>
MatrixVector<T> MatrixVector<T>::copy(const MatrixVector<T>& other) {
    MatrixVector<T> copy;
    size_t size = copy.size();
    for (size_t i = *other.e_start; i < size; i++) {
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

// ! IMPORTANT - let's be clear, the user has control and needs to ensure matrix is structured properly when populating...
template<typename T>
size_t Matrix<T>::rows() {
    return matrix.size();
}
template<typename T>
size_t Matrix<T>::columns() {
    if (this->rows() == 0) {
        return 0;
    }
    return (*this)[0].size();
}
// matrix copy constructor but not actual copy constructor lol... 
template<typename T>
Matrix<T> Matrix<T>::copy(const Matrix<T>& other) {
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
template<typename T>
Matrix<T> Matrix<T>::view(size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
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