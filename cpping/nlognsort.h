#ifndef NLOGNSORT_H
#define NLOGNSORT_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <gvc.h>
#include <cgraph.h>

#include <string>

static size_t nodes_visited = 0;

template<typename T>
void merge(T * A, size_t sizeA, T * B, size_t sizeB);

template<typename T>
void merge(T * A, size_t sizeA, T * B, size_t sizeB, T ** merged_container);

bool nodeWithKeyExists(Agraph_t * g, const char * key) {
    return agnode(g, (char *)key, 0) != nullptr;
}

Agnode_t * createUniqueNodeWithKey(Agraph_t * g, std::string key, std::string style) {
    if (style == "spaces") {
        printf("spaces...: %s\n", key.c_str());
        while (nodeWithKeyExists(g, key.c_str())) {
            key = " " + key + " ";
            printf("spaces...: %s\n", key.c_str());
        }
    } else if (style == "id") {
        while (nodeWithKeyExists(g, key.c_str())) {
            size_t id_i = key.find_first_of("_");
            int id = 0;
            if (id_i != std::string::npos) {
                id = atoi(key.substr(id_i + 1).c_str()) + 1;
            }
            char newKey[22];
            sprintf(newKey, "%s_%d", key.substr(0, id_i).c_str(), id);
            key = newKey;
        }
    }
    return agnode(g, (char *)key.c_str(), 1);
}

template<typename T>
std::string nodeStringFromBuffer(T * e_buf, size_t size) {
    std::string node_str;
    for (size_t i = 0; i < size; i++) {
        char int_char[10];
        sprintf(int_char, "%d", e_buf[i]);
        node_str += int_char;
        if (i < size - 1)
            node_str += ", ";
    }
    return node_str;
}

// lol... huh?
template<typename T>
Agnode_t * drawNode(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
    Agnode_t * node = createUniqueNodeWithKey(g, nodeStringFromBuffer(e_buf, size), "id"); 
    if (parent_node != nullptr) {
        agedge(g, parent_node, node, 0, 1);
    }
    return node;
}

template<typename T>
Agnode_t * drawMergedNode(Agraph_t * g, Agnode_t * left, Agnode_t * right, T * e_buf, size_t size) {
    Agnode_t * node = createUniqueNodeWithKey(g, nodeStringFromBuffer(e_buf, size), "id"); 
    if (left != nullptr) {
        agedge(g, left, node, 0, 1);
    }
    if (right != nullptr) {
        agedge(g, right, node, 0, 1);
    }
    return node;
}

// TODO: sort order...
// nlognsort
template<typename T>
Agnode_t * nlognSort(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
    if (e_buf == nullptr || size < 1) {
        return nullptr;
    } else if (size == 1) {
        return drawNode<T>(g, parent_node, e_buf, size);
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        printf("CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;

        // handle middle row properly... no need to include single node twice. If middle row, use this parent_node as parent for left/right_merged.
        //  unless, it's 2/1 split... let's include 2 nodes on the right hand side to keep things more symmetrical.
        Agnode_t * left = parent_node;
        Agnode_t * right = parent_node;
        if (size_left > 1) {
            left = drawNode<T>(g, parent_node, left_buf, size_left);
        }
        if (size_right > 1) {
            right = drawNode<T>(g, parent_node, right_buf, size_right);
        } else if (size_left == 2 && size_right == 1) {
            right = drawNode<T>(g, parent_node, right_buf, size_right);
            right = drawNode<T>(g, right, right_buf, size_right);
        }
        Agnode_t * left_sorted = nlognSort<T>(g, left, left_buf, size_left); // left
        Agnode_t * right_sorted = nlognSort<T>(g, right, right_buf, size_right); // right
        merge<T>(left_buf, size_left, right_buf, size_right);
        printf("CALL TRACE merged size: %ld\n", size);
        return drawMergedNode<T>(g, left_sorted, right_sorted, e_buf, size);
    }
}

template<typename T>
void nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;
        printf("CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, size_left, size_right);
        nlognSort<T>(left_buf, size_left); // left
        nlognSort<T>(right_buf, size_right); // right
        merge<T>(left_buf, size_left, right_buf, size_right);
        printf("CALL TRACE merged size: %ld\n", size);
    }
}

// nlognsort-normal
template<typename T>
void nlognSort(T * e_buf, size_t size, T ** merged_container) {
    if (e_buf == nullptr || size <= 1) {
        *merged_container = e_buf;
    } else {
        // ensure left always larger than right
        size_t size_left = ceil(size/2.0);
        size_t size_right = size - size_left;
        T * left_buf = e_buf;
        T * right_buf = e_buf + size_left;

        T * A;
        nlognSort<T>(left_buf, size_left, &A); // left
        T * B;
        nlognSort<T>(right_buf, size_right, &B); // right
        printf("invalid pointer\n");
        merge<T>(A, size_left, B, size_right, merged_container);
    }
}

void generateRandomArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        // array[i] = random();
        // normalized to size of array for easier visualization
        array[i] = (int)round(size * (double)rand()/RAND_MAX);
    }
}

void printArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("[%d]", array[i]);
    }
    printf("\n");
}

GVC_t * graphInit(int argc, char **argv) {
    GVC_t *gvc;
    /* set up a graphviz context */
    gvc = gvContext();
    /* parse command line args - minimally argv[0] sets layout engine */
    for (size_t i = 0; i < argc; i++) {
        printf("argv[%ld]: %s\n", i, argv[i]);
    }
    gvParseArgs(gvc, argc, argv);

    return gvc;
}

#endif