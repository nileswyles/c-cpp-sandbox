#ifndef WYLESLIBS_DATASTRUCTURES_H
#define WYLESLIBS_DATASTRUCTURES_H

#include <map>
#include <string>
#include <memory>

#include "datastructures/array.h"

namespace WylesLibs::DS {
    template<typename T>
    class Matrix {
        public:
            Array<Array<T>> matrix;
            // TODO: how to set limit/strict requirements? Do I care?
            Matrix() = default;
            ~Matrix() = default;
            size_t rows() {
                return matrix.size();
            }
            size_t columns() {
                // lol..
                return (*this)[0].size();
            }
            Array<T>& operator[] (const size_t pos) {
                return matrix[pos];
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