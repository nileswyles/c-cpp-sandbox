#include "tester.h"
#include "parser/csv/csv.h"

#ifndef LOGGER_CSV_TEST
#define LOGGER_CSV_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_CSV_TEST

using namespace WylesLibs;
using namespace WylesLibs::Test;

static void testMatrix(TestArg * t) {
}

static void testMatrixView(TestArg * t) {

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

    t.addTest(testCSVParser);
    t.addTest(testCSVParserSkipHeader);
    t.addTest(testCSVParserDelimeter);
    t.addTest(testCSVParserRecordWithNoFields);
    t.addTest(testCSVParserRecordWithInvalidNumberOfFields);
    t.addTest(testCSVParserRecordWithSpaces);
    t.addTest(testCSVParserRecordWithQuotes);
    t.addTest(testCSVParserRecordWithNestedQuotes);
    t.addTest(testCSVParserFromFileAll);
    t.addTest(testCSVParserFromFileRange);

    // TODO: catch exceptions so that before and after suite is still performed?
    //      Where too? in run function?
    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}