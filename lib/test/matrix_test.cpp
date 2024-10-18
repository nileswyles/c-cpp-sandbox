#include "tester.h"
#include "parser/csv/csv.h"

#ifndef LOGGER_CSV_TEST
#define LOGGER_CSV_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_CSV_TEST

using namespace WylesLibs;
using namespace WylesLibs::Test;

// yeah because we certaintly don't want it reallocating everything... and this way you don't need any pointers here...
//  alternative would be to have MatrixVector extend Array instead of SharedArray and mem like a normal person lol...

// static void printMatrix(Matrix<uint8_t>& matrix) {
static void printMatrix(Matrix<uint8_t> matrix) {
    for (size_t i = 0; i < matrix.rows(); i++) {
        for (size_t j = 0; j < matrix.columns(); j++) {
            printf("%u", matrix[i][j]);
            if (j+1 != matrix.columns()) {
                printf(",");
            }
        }
        printf("\n");
    }
}

static void testMatrix(TestArg * t) {
    Matrix<uint8_t> matrix;
    // 2x2
    size_t val = 0;
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < 2; j++) {
            matrix[i][j] = ++val;
        }
    }

    printMatrix(matrix);
}

static void testMatrixImmutableView(TestArg * t) {
    // 4x4
    // ****
    // ****
    // ****
    // ****
    Matrix<uint8_t> matrix{
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    printf("Matrix: \n");
    printMatrix(matrix);
}

static void testMatrixView(TestArg * t) {
    Matrix<uint8_t> matrix;
    // 4x4
    // ****
    // ****
    // ****
    // ****
    size_t val = 0;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            matrix[i][j] = ++val;
        }
    }
    printf("Matrix: \n");
    printMatrix(matrix);

    printf("Matrix View: \n");
    printMatrix(matrix.view(1,4,1,3));
}

static void testMatrixViewRemovedElements(TestArg * t) {
    Matrix<uint8_t> matrix;
    // 4x4
    // ****
    // ****
    // ****
    // ****
    size_t val = 0;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            matrix[i][j] = ++val;
        }
    }
    printf("Matrix: \n");
    printMatrix(matrix);

    printf("Matrix View: \n");
    printMatrix(matrix.view(1,4,1,3));
}
static void testMatrixViewAddedElements(TestArg * t) {
    Matrix<uint8_t> matrix;
    // 4x4
    // ****
    // ****
    // ****
    // ****
    size_t val = 0;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            matrix[i][j] = ++val;
        }
    }
    printf("Matrix: \n");
    printMatrix(matrix);

    printf("Matrix View: \n");
    printMatrix(matrix.view(1,4,1,3));
}
static void testMatrixViewOneElement(TestArg * t) {
    Matrix<uint8_t> matrix;
    // 4x4
    // ****
    // ****
    // ****
    // ****
    size_t val = 0;
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 4; j++) {
            matrix[i][j] = ++val;
        }
    }
    printf("Matrix: \n");
    printMatrix(matrix);

    printf("Matrix View: \n");
    printMatrix(matrix.view(1,4,1,3));
}
static void testMatrixCopy(TestArg * t) {

}
static void testMatrixViewOfView(TestArg * t) {

}

static void testMatrixViewOfCopy(TestArg * t) {

}

static void testMatrixCopyOfView(TestArg * t) {

}

static void testMatrixCopyOfCopy(TestArg * t) {

}

// TODO: how to test containerized stuff..

int main(int argc, char * argv[]) {
    Tester t("Matrix Tests");

    t.addTest(testMatrix);
    t.addTest(testMatrixView);
    // t.addTest(testCSVParserSkipHeader);
    // t.addTest(testCSVParserDelimeter);
    // t.addTest(testCSVParserRecordWithNoFields);
    // t.addTest(testCSVParserRecordWithInvalidNumberOfFields);
    // t.addTest(testCSVParserRecordWithSpaces);
    // t.addTest(testCSVParserRecordWithQuotes);
    // t.addTest(testCSVParserRecordWithNestedQuotes);
    // t.addTest(testCSVParserFromFileAll);
    // t.addTest(testCSVParserFromFileRange);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}