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
            MatrixVector(): e_start(new size_t(0)), e_end(new size_t(0)) {}
            MatrixVector(const MatrixVector<T>& other, size_t start, size_t end): Array<T>(other.instance_count, other.e_buf, other.e_cap, other.e_size, other.e_sorted) {
                // view...
                (*other.instance_count)++;

                size_t view_size = end - start;
                if (view_size == 0 || view_size > *other.e_size) {
                    throw std::runtime_error("Invalid view coordinates.");
                }
                e_start = new size_t(start); 
                e_end = new size_t(end); 
            }
            virtual ~MatrixVector() {
                if (false == this->destructed) {
                    (*this->instance_count)--;
                    if (*this->instance_count == 0) {
                        deleteCArray<T>(this->e_buf, *this->e_size);
                        if (this->instance_count != nullptr) {
                            delete this->instance_count;
                        }
                        if (this->e_cap != nullptr) {
                            delete this->e_cap;
                        } 
                        if (this->e_size != nullptr) {
                            delete this->e_size;
                        }
                        if (this->e_sorted != nullptr) {
                            delete this->e_sorted;
                        }
                        if (this->e_start != nullptr) {
                            delete this->e_start;
                        } 
                        if (this->e_end != nullptr) {
                            delete this->e_end;
                        }
                    }
                    this->destructed = true;
                }
            }
            // copy constructor - containerization code remains here
            MatrixVector(const MatrixVector<T>& other) {
                if (false == this->constructed) {
                    this->instance_count = other.instance_count;
                    this->e_buf = other.e_buf;
                    this->e_cap = other.e_cap;
                    this->e_size = other.e_size;
                    this->e_sorted = other.e_sorted;
                    this->e_start = other.e_start;
                    this->e_end = other.e_end;
             
                    (*this->instance_count)++;
                    this->constructed = true;
                }
            }
            // because not same as copy constructor
            MatrixVector<T> copy(const MatrixVector<T>& other);

            MatrixVector<T>& operator+ (const MatrixVector<T>& m);
            MatrixVector<T>& operator- (const MatrixVector<T>& m);
            MatrixVector<T>& operator* (const MatrixVector<T>& m);
            T& operator[] (const size_t pos);
            // copy assignment - containerization code remains here
            MatrixVector<T>& operator= (const MatrixVector<T>& other) {
                if (false == this->constructed) {
                    this->instance_count = other.instance_count;
                    this->e_buf = other.e_buf;
                    this->e_cap = other.e_cap;
                    this->e_size = other.e_size;
                    this->e_sorted = other.e_sorted;
                    this->e_start = other.e_start;
                    this->e_end = other.e_end;
             
                    // TODO: so, if default constructor of parent is called after this than that's an issue... and need to implement some extra check...
                    (*this->instance_count)++;
                    this->constructed = true;
                }

                return *this;
            }
    };
    template<typename T>
    class Matrix {
        protected:
            MatrixVector<MatrixVector<T>> matrix;
            size_t * instance_count;
            size_t * e_x_start;
            size_t * e_x_end;
            size_t * e_y_start;
            size_t * e_y_end;
        public:
            Matrix(size_t * instance_count, size_t x_start, size_t x_end, size_t y_start, size_t y_end) {
                if (instance_count == nullptr) {
                    instance_count = new size_t(1);
                } else {
                    instance_count = instance_count;
                    (*instance_count)++;
                }
                // LMAOOOOOO
                e_x_start = new size_t(x_start);
                e_x_end = new size_t(x_end);
                e_y_start = new size_t(y_start);
                e_y_end = new size_t(y_end);
            }
            Matrix(): Matrix(nullptr, 0, 0, 0, 0) {}
            virtual ~Matrix() {
                (*this->instance_count)--;
                if (*this->instance_count == 0) {
                    if (instance_count != nullptr) {
                        delete instance_count;
                    }
                    if (e_x_start != nullptr) {
                        delete e_x_start;
                    } 
                    if (e_x_end != nullptr) {
                        delete e_x_end;
                    }
                    if (e_y_start != nullptr) {
                        delete e_y_start;
                    } 
                    if (e_y_end != nullptr) {
                        delete e_y_end;
                    }
                }
            }
            // copy constructor - containerization code remains here
            Matrix(const Matrix<T>& other) {
                // TODO:
                // no access to private members? lol...
                this->instance_count = other.instance_count;
                this->e_y_start = other.e_y_start;
                this->e_y_end = other.e_y_end;
                this->e_x_start = other.e_x_start;
                this->e_x_end = other.e_x_end;
     
                (*this->instance_count)++;
            }
            size_t rows();
            size_t columns();
            // matrix copy constructor but not actual copy constructor lol... 
            Matrix<T> copy(const Matrix<T>& other);
            Matrix<T> view(size_t x_start, size_t x_end, size_t y_start, size_t y_end);
            Matrix<T>& operator+ (const Matrix<T>& m);
            Matrix<T>& operator- (const Matrix<T>& m);
            Matrix<T>& operator* (const Matrix<T>& m);
            MatrixVector<T>& operator[] (const size_t pos);
            // copy assignment - containerization code remains here
            Matrix<T>& operator= (const Matrix<T>& other) {
                this->instance_count = other.instance_count;
                this->e_y_start = other.e_y_start;
                this->e_y_end = other.e_y_end;
                this->e_x_start = other.e_x_start;
                this->e_x_end = other.e_x_end;
     
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