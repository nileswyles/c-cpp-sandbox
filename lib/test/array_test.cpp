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

// TODO:
//  Template and convert to uint8_t in assert function?
//  Revisit and think about this some more later.
typedef struct ArrayAssert {
    uint8_t * actual;
    size_t actual_size;
    size_t actual_cap;
    uint8_t * expected;
    size_t expected_size;
    size_t expected_cap;
} ArrayAssert;

// bool assert(Array * arr, Array * expected_arr) {
// bool assert(ArrayAssert assert) {
//     bool memory_match = memcmp(assert.expected, assert.actual, assert.expected_size) == 0;
//     bool size_match = assert.actual_size == assert.expected_size;
//     // make sure cap grows at the predetermined rate.
//     // also make sure size doesn't exceed cap (arguably more important).
//     bool cap_match = assert.actual_cap == assert.expected_cap && assert.actual_size <= assert.actual_cap; 

//     loggerPrintf(LOGGER_TEST, "Expected:\n");
//     loggerPrintByteArray(LOGGER_TEST, assert.expected, assert.expected_size * expected_size_of_el);
//     loggerPrintf(LOGGER_TEST, "Actual:\n");
//     loggerPrintByteArray(LOGGER_TEST, assert.actual, assert.actual_size * expected_size_of_el);
//     loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (%lu == %lu), Cap Match: %s (%lu == %lu), Size of El Match: %s (%lu == %lu) \n", 
//         memory_match ? "True" : "False", 
//         size_match ? "True" : "False", assert.expected_size, assert.actual_size,
//         cap_match ? "True" : "False", assert.expected_cap assert.actual_cap
//     );

//     if (!memory_match || !size_match || !cap_match) {
//         printf("\nTest Failed!\n");
//     } else {
//         printf("\nTest Passed!\n");
//     }

//     return memory_match && size_match && cap_match;
// }

// bool assert(Array * arr, Array * expected_arr) {
bool assert(Array<uint8_t> * arr, void * expected, size_t expected_size, size_t expected_cap, size_t expected_size_of_el) {
    bool memory_match = memcmp(expected, (uint8_t *)arr->buf, expected_size) == 0;
    bool size_match = arr->getSize() == expected_size;
    // make sure cap grows at the predetermined rate.
    // also make sure size doesn't exceed cap (arguably more important).

    bool cap_match = true;
    // bool cap_match = arr->cap == expected_cap && arr->size <= arr->cap; 
    // bool size_of_el_match = expected_size_of_el == arr->size_of_el;

    loggerPrintf(LOGGER_TEST, "Expected:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)expected, expected_size * expected_size_of_el);
    loggerPrintf(LOGGER_TEST, "Actual:\n");
    loggerPrintByteArray(LOGGER_TEST, (uint8_t *)arr->buf, arr->getSize() * expected_size_of_el);
    loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (%lu == %lu)\n", 
        memory_match ? "True" : "False", 
        size_match ? "True" : "False", expected_size, arr->getSize()
    );
    // loggerPrintf(LOGGER_TEST, "Memory Match: %s, Size Match: %s (%lu == %lu), Cap Match: %s (%lu == %lu), Size of El Match: %s (%lu == %lu) \n", 
    //     memory_match ? "True" : "False", 
    //     size_match ? "True" : "False", expected_size, arr->size 
        // cap_match ? "True" : "False", expected_cap, arr->cap,
        // size_of_el_match ? "True" : "False", expected_size_of_el, arr->size_of_el
    // );

    if (!memory_match || !size_match || !cap_match) {
        printf("\nTest Failed!\n");
    } else {
        printf("\nTest Passed!\n");
    }

    return memory_match && size_match && cap_match;
}

bool test_array_append() {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", __func__);
    Array<uint8_t> arr;

    // VLA!
    size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP - 1; // in this case, expected_size == expected_num_els
    uint8_t expected[expected_size];
    memset(expected, 0x07, expected_size);

    arr.append(expected, expected_size);

    bool res = assert(&arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));

    printf("\n#######################################\n");

    return res;
}

bool test_array_append_cstrings() {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", __func__);
    Array<const char *> arr(ARRAY_RECOMMENDED_INITIAL_CAP);
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
    // char * expected[7] = {
    //     "STRING 1",
    //     "STRING 2",
    //     "STRING 3",
    //     "STRING 4",
    //     "STRING 5",
    //     "STRING 6",
    //     "STRING 7"
    // };

    arr.append("STRING!!!!");
    arr.append(expected, expected_size);
   // bool res = assert(&arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));
   bool res = true;

    // printf("WTF?, %d, %x\n", arr.getSize(), arr.buf[0]);
   for (size_t i = 0; i < arr.getSize(); i++) {
        loggerPrintf(LOGGER_TEST, "%s, %p, %p\n", arr[i], &(arr[i]), &(arr.buf[i]));
   }

    printf("\n#######################################\n");

    return res;
}

// bool test_array_append_resize() {
//     printf("\n#######################################\n");
//     printf("\nTest Func: %s\n\n", __func__);
//     Array * arr = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));

//     size_t expected_size = ARRAY_RECOMMENDED_INITIAL_CAP + 7; // in this case, expected_size == expected_num_els - force resize
//     uint8_t expected[expected_size];
//     memset(expected, 0x07, expected_size);

//     size_t initial_size = arr->size;
//     array_append(arr, expected, expected_size);

//     bool res = assert(arr, (void *)expected, expected_size, (size_t)((expected_size + initial_size) * 1.75), sizeof(uint8_t)); // only one resize performed

//     printf("\n#######################################\n");

//     array_destructor(arr);

//     return res;
// }

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
    test_array_append_cstrings();
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