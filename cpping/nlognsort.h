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

template<typename T>
Agnode_t * drawMultiValueNode(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
    std::string node_str;
    for (size_t i = 0; i < size; i++) {
        char int_char[10];
        sprintf(int_char, "%d", e_buf[i]);
        node_str += int_char;
        if (i < size - 1)
            node_str += " ";
    }
    Agnode_t * node = agnode(g, (char *)node_str.c_str(), 1);
    if (parent_node != nullptr) {
        agedge(g, parent_node, node, 0, 1);
    }
    return node;
}

template<typename T>
Agnode_t * drawMergedNode(Agraph_t * g, Agnode_t * left, Agnode_t * right, T * e_buf, size_t size) {
    std::string node_str;
    for (size_t i = 0; i < size; i++) {
        char int_char[10];
        sprintf(int_char, "%d", e_buf[i]);
        node_str += int_char;
        // if (i < size - 1)
            node_str += " ";
    }
    Agnode_t * node = agnode(g, (char *)node_str.c_str(), 1);
    if (left != nullptr) {
        agedge(g, left, node, 0, 1);
    }
    if (right != nullptr) {
        agedge(g, right, node, 0, 1);
    }
    return node;
}

template<typename T>
Agnode_t * drawNode(Agraph_t * g, Agnode_t * parent_node, T e_buf) {
    char int_char[11];
    sprintf(int_char, "%d ", e_buf);
    std::string node_str(int_char);
    Agnode_t * node = agnode(g, (char *)node_str.c_str(), 1);
    agedge(g, parent_node, node, 0, 1);
    return node;
}

// TODO: sort order...

// nlognsort
template<typename T>
Agnode_t * nlognSort(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return drawNode(g, parent_node, e_buf[0]);
    } else {
        size_t split_index = size/2;
        printf("CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, split_index, size-split_index);
        Agnode_t * left = nullptr;
        Agnode_t * right = nullptr;
        left = drawMultiValueNode(g, parent_node, e_buf, split_index);
        right = drawMultiValueNode(g, parent_node, e_buf + split_index, size - split_index);
        Agnode_t * left_merged = nlognSort<T>(g, left, e_buf, split_index); // left
        Agnode_t * right_merged = nlognSort<T>(g, right, e_buf + split_index, size - split_index); // right
        merge<T>(e_buf, split_index, e_buf + split_index, size - split_index);
        printf("CALL TRACE merged size: %ld\n", size);
        return drawMergedNode<T>(g, left_merged, right_merged, e_buf, size);
    }
}

template<typename T>
void nlognSort(T * e_buf, size_t size) {
    if (e_buf == nullptr || size <= 1) {
        return;
    } else {
        size_t split_index = size/2;
        printf("CALL TRACE: size: %ld, left: %ld, right: %ld\n", size, split_index, size-split_index);
        nlognSort<T>(e_buf, split_index); // left
        nlognSort<T>(e_buf + split_index, size - split_index); // right
        merge<T>(e_buf, split_index, e_buf + split_index, size - split_index);
        printf("CALL TRACE merged size: %ld\n", size);
    }
}

// nlognsort-normal
template<typename T>
void nlognSort(T * e_buf, size_t size, T ** merged_container) {
    if (e_buf == nullptr || size <= 1) {
        *merged_container = e_buf;
    } else {
        size_t split_index = size/2;
        T * A;
        nlognSort<T>(e_buf, split_index, &A); // left
        T * B;
        nlognSort<T>(e_buf + split_index, size - split_index, &B); // right
        printf("invalid pointer\n");
        merge<T>(A, split_index, B, size - split_index, merged_container);
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