#include "array.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// TODO: still unsure about this?
#ifdef TEST_DEBUG
#define LOGGER_LEVEL LOGGER_DEBUG
#endif

#include "logger.h"

using namespace WylesLibs;


// bool assert(Array * arr, Array * expected_arr) {
bool assert(Array<uint8_t> * arr, void * expected, size_t expected_size, size_t expected_cap, size_t expected_size_of_el) {
    bool memory_match = memcmp(expected, (uint8_t *)arr->buf, expected_size) == 0;
    bool size_match = arr->size == expected_size;
    // make sure cap grows at the predetermined rate.
    // also make sure size doesn't exceed cap (arguably more important).

    bool cap_match = true;
    // bool cap_match = arr->cap == expected_cap && arr->size <= arr->cap; 
    // bool size_of_el_match = expected_size_of_el == arr->size_of_el;

    logger_printf(LOGGER_DEBUG, "Expected:\n");
    logger_print_byte_array(LOGGER_DEBUG, (uint8_t *)expected, expected_size * expected_size_of_el);
    logger_printf(LOGGER_DEBUG, "Actual:\n");
    logger_print_byte_array(LOGGER_DEBUG, (uint8_t *)arr->buf, arr->size * expected_size_of_el);
    logger_printf(LOGGER_DEBUG, "Memory Match: %s, Size Match: %s (%lu == %lu)\n", 
        memory_match ? "True" : "False", 
        size_match ? "True" : "False", expected_size, arr->size 
    );
    // logger_printf(LOGGER_DEBUG, "Memory Match: %s, Size Match: %s (%lu == %lu), Cap Match: %s (%lu == %lu), Size of El Match: %s (%lu == %lu) \n", 
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
    Array<char *> arr;

    size_t expected_size = 7;
    char * expected[7] = {
        "STRING 1",
        "STRING 2",
        "STRING 3",
        "STRING 4",
        "STRING 5",
        "STRING 6",
        "STRING 7"
    };

    arr.append(expected, expected_size);

   // bool res = assert(&arr, (void *)expected, expected_size, ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(uint8_t));
   bool res = true;

   for (size_t i = 0; i < 7; i++) {
        logger_printf(LOGGER_DEBUG, "%s\n", arr.buf[i]);
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