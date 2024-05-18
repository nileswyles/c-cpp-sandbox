#ifndef RESULT_H
#define RESULT_H

#if defined __cplusplus
extern "C"
{
#endif

typedef enum operation_result {
    OPERATION_SUCCESS, // 0
    OPERATION_ERROR,
    MEMORY_OPERATION_ERROR,
    IO_OPERATION_ERROR,
} operation_result;

#if defined __cplusplus
}
#endif

#endif