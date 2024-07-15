#ifndef NLOGNSORT_H
#define NLOGNSORT_H

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <gvc.h>
#include <cgraph.h>

#include <string>
#include <unordered_map>

#include "logger.h"

#ifndef GRAPH_ENABLE
#define GRAPH_ENABLE 1
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 4
#endif

static constexpr const char * UNIQUE_KEY_STYLE = "spaces";

static size_t nodes_visited = 0;

bool nodeWithKeyExists(Agraph_t * g, const char * key) {
    return agnode(g, (char *)key, 0) != nullptr;
}

Agnode_t * createUniqueNodeWithKey(Agraph_t * g, std::string key, std::string style) {
    if (style == "spaces") {
        loggerPrintf(LOGGER_DEBUG_VERBOSE, "spaces...: %s\n", key.c_str());
        while (nodeWithKeyExists(g, key.c_str())) {
            key = " " + key + " ";
            loggerPrintf(LOGGER_DEBUG_VERBOSE, "spaces...: %s\n", key.c_str());
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
    loggerPrintf(LOGGER_DEBUG, "%s\n", node_str.c_str());
    return node_str;
}

// lol... huh?
template<typename T>
Agnode_t * drawNode(Agraph_t * g, Agnode_t * parent_node, T * e_buf, size_t size) {
    Agnode_t * node = createUniqueNodeWithKey(g, nodeStringFromBuffer(e_buf, size), UNIQUE_KEY_STYLE); 
    if (parent_node != nullptr) {
        agedge(g, parent_node, node, 0, 1);
    }
    return node;
}

template<typename T>
Agnode_t * drawMergedNode(Agraph_t * g, Agnode_t * left, Agnode_t * right, T * e_buf, size_t size) {
    Agnode_t * node = createUniqueNodeWithKey(g, nodeStringFromBuffer(e_buf, size), UNIQUE_KEY_STYLE); 
    if (left != nullptr) {
        agedge(g, left, node, 0, 1);
    }
    if (right != nullptr) {
        agedge(g, right, node, 0, 1);
    }
    return node;
}

template<typename T>
void drawUnsorted(Agraph_t * g, Agnode_t * parent_node, Agnode_t ** left, Agnode_t ** right, T * left_buf, size_t size_left, T * right_buf, size_t size_right) {
    // handle middle row properly... no need to include single node twice. If middle row, use this parent_node as parent for left/right_merged.
    //  unless, it's 2/1 split... let's include 2 nodes on the right hand side to keep things more symmetrical.
    *left = parent_node;
    *right = parent_node;
    if (size_left > 1) {
        *left = drawNode<T>(g, parent_node, left_buf, size_left);
    }
    if (size_right > 1) {
        *right = drawNode<T>(g, parent_node, right_buf, size_right);
    } else if (size_left == 2 && size_right == 1) {
        *right = drawNode<T>(g, parent_node, right_buf, size_right);
        *right = drawNode<T>(g, *right, right_buf, size_right);
    }
}

void generateRandomArray(int * array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        // array[i] = random();
        // normalized to size of array for easier visualization
        array[i] = (int)round(size * (double)random()/RAND_MAX);
    }
}

void printArray(int * array, size_t size) {
    if (LOGGER_LEVEL >= LOGGER_TEST_VERBOSE) {
        for (size_t i = 0; i < size; i++) {
            printf("[%d]", array[i]);
        }
        printf("\n");
    }
}

GVC_t * graphInit(int argc, char **argv) {
    GVC_t *gvc;
    /* set up a graphviz context */
    gvc = gvContext();
    /* parse command line args - minimally argv[0] sets layout engine */
    for (size_t i = 0; i < argc; i++) {
        loggerPrintf(LOGGER_DEBUG, "argv[%ld]: %s\n", i, argv[i]);
    }
    gvParseArgs(gvc, argc, argv);

    return gvc;
}

template<typename T>
bool compareArrays(T * A, size_t sizeA, T * B, size_t sizeB) {
    // Checks whether array B contains the same number of each unique element in A (order doesn't matter). 
    //  "Same number of each unique element" implies we're accounting for duplicates...

    if (sizeA == 0 || sizeA != sizeB) {
        return false;
    }

    // value, count
    std::unordered_map<size_t, size_t> A_count;
    std::unordered_map<size_t, size_t> B_count;

    bool match = true;
    T prev = B[0];
    for (size_t i = 0; i < sizeA; i++) {
        if (!A_count.contains(A[i])) {
            A_count[A[i]] = 0;
        } else {
            A_count[A[i]]++;
        }
        // B must be sorted...
        if (prev > B[i]) {
            return false;
        }
        if (!B_count.contains(B[i])) {
            B_count[B[i]] = 0;
        } else {
            B_count[B[i]]++;
        }
        prev = B[i];
    }

    if (A_count.size() != B_count.size()) {
        return false;
    }

    for (const auto& [key, value] : A_count) {
        if (value != B_count[key]) {
            return false;
        }
    }

    return true;
}

#endif