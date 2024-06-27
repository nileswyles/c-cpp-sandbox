#include "array.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <string>
#include <stdio.h>

#ifndef LOGGER_LEVEL 
#define LOGGER_LEVEL LOGGER_TEST
#endif

#include "logger.h"

using namespace WylesLibs;

template<typename T>
bool assert(Array<T> actual, T * expected, size_t expected_size, size_t expected_cap) {
    bool memory_match = memcmp((void *)expected, (void *)actual.buf(), expected_size * sizeof(T)) == 0;
    bool size_match = actual.size() == expected_size;
    // make sure cap grows at the predetermined rate.
    // also make sure size doesn't exceed cap (arguably more important).

    bool cap_match = actual.cap() == expected_cap;
    loggerPrintf(LOGGER_TEST, "Expected:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)expected, expected_size * sizeof(T));
    loggerPrintf(LOGGER_TEST, "Actual:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)actual.buf(), actual.size() * sizeof(T));
    loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (%lu == %lu), Cap Match: %s (%lu == %lu)\n", 
        memory_match ? "True" : "False", 
        size_match ? "True" : "False", expected_size, actual.size(),
        cap_match ? "True" : "False", expected_cap, actual.cap()
    );

    if (!memory_match || !size_match || !cap_match) {
        printf("\nTest Failed!\n");
    } else {
        printf("\nTest Passed!\n");
    }

    return memory_match && size_match && cap_match;
}

// template<>
// bool assert<const char *>(Array<const char *> actual, const char ** expected, size_t expected_size, size_t expected_cap) {
//     bool size_match = actual.size() == expected_size;
//     // make sure cap grows at the predetermined rate.
//     // also make sure size doesn't exceed cap (arguably more important).
//     bool cap_match = actual.cap() == expected_cap;

//     bool memory_match = true;
//     if (size_match) {
//         for (size_t i = 0; i < expected_size; i++) {
//             if (strcmp(expected[i], actual.buf()[i]) != 0) {
//                 memory_match = false;
//             }
//         }
//     }
//     loggerPrintf(LOGGER_TEST, "Expected:\n");
//     for (size_t i = 0; i < expected_size; i++) {
//         loggerPrintf(LOGGER_TEST, "%s\n", expected[i]);
//     }
//     loggerPrintf(LOGGER_TEST, "Actual:\n");
//     for (size_t i = 0; i < actual.size(); i++) {
//         loggerPrintf(LOGGER_TEST, "%s\n", actual.buf()[i]);
//     }
//     loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (%lu == %lu), Cap Match: %s (%lu == %lu)\n", 
//         memory_match ? "True" : "False", 
//         size_match ? "True" : "False", expected_size, actual.size(),
//         cap_match ? "True" : "False", expected_cap, actual.cap()
//     );

//     if (!memory_match || !size_match || !cap_match) {
//         printf("\nTest Failed!\n");
//     } else {
//         printf("\nTest Passed!\n");
//     }

//     return memory_match && size_match && cap_match;
// }

bool test_array_append_cstrings() {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", __func__);

    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1;
    const char * expected[ARRAY_RECOMMENDED_INITIAL_CAP] = {
        std::string("STRING 1").c_str(),
        std::string("STRING 2").c_str(),
        std::string("STRING 3").c_str(),
        std::string("STRING 4").c_str(),
        std::string("STRING 5").c_str(),
        std::string("STRING 6").c_str(),
        std::string("STRING 7").c_str()
    };
    // Array<const char *> actual(ARRAY_RECOMMENDED_INITIAL_CAP);
    // actual.append(expected, expected_size);

    // bool res = assert<const char *>(actual, expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP);

    printf("\n#######################################\n");

    // return res;
    return 1;
}

bool test_array_append() {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", __func__);

    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
    uint8_t expected[expected_size];
    memset(expected, 0x07, expected_size);
    
    Array<uint8_t> actual;
    actual.append(expected, expected_size);

    bool res = assert<uint8_t>(actual, expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP);

    printf("\n#######################################\n");

    return res;
}

bool test_array_append_resize() {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", __func__);

    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els - force resize
    uint8_t expected[expected_size];
    memset(expected, 0x07, expected_size);

    Array<uint8_t> actual;
    size_t initial_size = actual.size();
    actual.append(expected, expected_size);

    bool res = assert<uint8_t>(actual, expected, expected_size, (size_t)((expected_size + initial_size) * 1.75));

    printf("\n#######################################\n");

    return res;
}

// bool test_array_append_consecutive() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
//     uint8_t expected[expected_size];

//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0x07;
//         array_append(arr, expected + i, 1);
//     }

//     bool res = assert(arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

// bool test_array_append_consecutive_resize() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els - force resize
//     uint8_t expected[expected_size];

//     size_t initial_size = arr->size;

//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0x07;
//         array_append(arr, &expected[i], 1);
//     }

//     bool res = assert(arr, (void *)expected, expected_size, (size_t)(ARRAY_RECOMMENDED_INITIAL_CAP + 1) * 1.75, sizeof(uint8_t)); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

// bool test_array_append_large_el() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     size_t expected_size_of_el = sizeof(uint64_t);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el);

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
//     uint64_t expected[expected_size];

//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0xFFFFFFFFFFFFFF07;
//     }
    
//     array_append(arr, (void *)expected, expected_size);

//     bool res = assert(arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

// // is this even necessary??
// bool test_array_append_large_el_resize() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     size_t expected_size_of_el = sizeof(uint64_t);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el);

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els -- force resize
//     uint64_t expected[expected_size];

//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0xFFFFFFFFFFFFFF07;
//     }
    
//     size_t initial_size = arr->size;
//     array_append(arr, (void *)expected, expected_size);

//     bool res = assert(arr, (void *)expected, expected_size, (size_t)((expected_size + initial_size) * 1.75), expected_size_of_el); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

// bool test_array_append_large_el_consecutive() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     size_t expected_size_of_el = sizeof(uint64_t);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el);

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
//     uint64_t expected[expected_size];

//     size_t initial_size = arr->size;
//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0xFFFFFFFFFFFFFF07;
//         array_append(arr, (void *)&expected[i], 1);
//     }

//     bool res = assert(arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

// bool test_array_append_large_el_consecutive_resize() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     size_t expected_size_of_el = sizeof(uint64_t);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, expected_size_of_el);

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els -- force resize
//     uint64_t expected[expected_size];

//     size_t initial_size = arr->size;
//     for (int i = 0; i < expected_size; i++) {
//         expected[i] = 0xFFFFFFFFFFFFFF07;
//         array_append(arr, (void *)&expected[i], 1);
//     }

//     bool res = assert(arr, (void *)expected, expected_size, (size_t)((ARRAY_RECOMMENDED_INITIAL_CAP + 1) * 1.75), expected_size_of_el); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

int main() {
    // test_array_append_cstrings();
    test_array_append();
    // test_array_append_resize();
    // test_array_append_consecutive();
    // test_array_append_consecutive_resize();
    // test_array_append_large_el();
    // test_array_append_large_el_resize();
    // test_array_append_large_el_consecutive();
    // test_array_append_large_el_consecutive_resize();

    // test_array_insert();
    // test_array_insert_resize();
    // test_array_insert_consecutive();
    // test_array_insert_consecutive_resize();
    // test_array_insert_large_el();
    // test_array_insert_large_el_resize();
    // test_array_insert_large_el_consecutive();
    // test_array_insert_large_el_consecutive_resize();

    // test_array_remove();
    // test_array_remove_consecutive();
    // test_array_remove_large_el();
    // test_array_remove_large_el_consecutive();
}