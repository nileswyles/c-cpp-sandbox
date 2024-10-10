#include "tester.h"
#include "datastructures/array.h"

#ifndef LOGGER_CSV_TEST
#define LOGGER_CSV_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_CSV_TEST

using namespace WylesLibs;
using namespace WylesLibs::Test;

// see RFC4180....

static void testCSVParser(TestArg * t) {

}

static void testCSVParserSkipHeader(TestArg * t) {

}

static void testCSVParserDelimeter(TestArg * t) {

}
static void testCSVParserRecordWithNoFields(TestArg * t) {

}

static void testCSVParserRecordWithInvalidNumberOfFields(TestArg * t) {

}

static void testCSVParserRecordWithSpaces(TestArg * t) {

}

static void testCSVParserRecordWithQuotes(TestArg * t) {

}

static void testCSVParserRecordWithNestedQuotes(TestArg * t) {
}

static void testCSVParserFromFileAll(TestArg * t) {

}

static void testCSVParserFromFileRange(TestArg * t) {

}

int main(int argc, char * argv[]) {
    Tester t("CSV Parser Tests");

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