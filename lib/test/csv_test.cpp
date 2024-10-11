#include "tester.h"
#include "parser/csv/csv.h"

#ifndef LOGGER_CSV_TEST
#define LOGGER_CSV_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_CSV_TEST

using namespace WylesLibs;
using namespace WylesLibs::Test;

// see RFC4180....

static void testCSVParser(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += EOF;

    std::shared_ptr<IOStream> io = std::make_shared<IOStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSVParser p(io);
    CSV csv = p.read(true);
    // printf("%s\n", csv.toString().c_str());
    // for (size_t i = 0; i < csv.header.size(); i++) {
    //     printf("%s\n", csv.header[i].c_str());
    // }
    // for (size_t i = 0; i < csv.rows(); i++) {
    //     for (size_t j = 0; j < csv.columns(); j++) {
    //         printf("%s", csv[i][j].c_str());
    //         if (j + 1 == csv.columns()) {
    //             printf("\n");
    //         } else {
    //             printf(",");
    //         }
    //     }
    // }
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