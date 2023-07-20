#ifndef RESULT_H
#define RESULT_H

typedef enum operation_result {
    OPERATION_SUCCESS, // 0
    OPERATION_ERROR,
    MEMORY_OPERATION_ERROR,
    IO_OPERATION_ERROR,
} operation_result;

#endif
