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
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    csv.read(true);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "1:1" &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:2" &&
        csv[1][1] == "2:2") {
        t->fail = false;
    }
}

static void testCSVParserPeriodSeparator(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<std::string> csv(io, '.');
    } catch (std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Periods aren't allowed as CSV separator.", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserDoubles(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1.797,27\n";
    csv_string += "4,2\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<double> csv(io);
    csv.read(true);
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == 1.797 &&
        csv[0][1] == 27 &&
        csv[1][0] == 4 &&
        csv[1][1] == 2) {
        t->fail = false;
    }
}

static void testCSVParserDoublesNonNumber(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1.797,lkannlakn\n";
    csv_string += "4,2\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<double> csv(io);
        csv.read(true);
    } catch (std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Non-digit character found in CSV data.", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserDoublesRecordWithInvalidNumberOfFields(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1.797\n";
    csv_string += "4,2\n";
    csv_string += (char)EOF; // 255?

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<double> csv(io);
        csv.read(false);
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Invalid record size.", e.what()) == 0) {
            t->fail = false;
        }
    }
}


static void testCSVParserSkipHeader(TestArg * t) {
    std::string csv_string("1:1,2:1\n");
    csv_string += "1:2,2:2\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    csv.read(false);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    if (csv.header.size() == 0 &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "1:1" &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:2" &&
        csv[1][1] == "2:2") {
        t->fail = false;
    }
}

static void testCSVParserDelimeter(TestArg * t) {
    std::string csv_string("col_1|col_2\n");
    csv_string += "1:1|2:1\n";
    csv_string += "1:2|2:2\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io, '|');
    csv.read(true);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "1:1" &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:2" &&
        csv[1][1] == "2:2") {
        t->fail = false;
    }
}

static void testCSVParserRecordWithNoFields(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "\n";
    csv_string += "1:2,2:2\n";
    csv_string += (char)EOF; // 255?

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<std::string> csv(io);
        csv.read(false);
        loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Invalid record size.", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserRecordWithInvalidNumberOfFields(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1:1,\n";
    csv_string += "1:2,2:2\n";
    csv_string += (char)EOF; // 255?

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<std::string> csv(io);
        csv.read(false);
        loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Invalid record size.", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserRecordWithSpaces(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "   1:1   ,2:1\n";
    csv_string += "1:2,   2:2   \n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    csv.read(true);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "   1:1   " &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:2" &&
        csv[1][1] == "   2:2   ") {
        t->fail = false;
    }
}

static void testCSVParserRecordWithQuotes(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "\"1:1\",2:1\n";
    csv_string += "1:2,\"2:2\"\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    csv.read(true);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "1:1" &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:2" &&
        csv[1][1] == "2:2") {
        t->fail = false;
    }
}

static void testCSVParserRecordWithNestedQuotes(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "\"1:\"\"somestring\"\"1\",2:1\n";
    csv_string += "\"1:\"\"somestring\"\"2\",\"2:2\"\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 2;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    csv.read(true);
    loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[%s]\n", csv.toString().c_str());
    if (csv.header[0] == "col_1" &&
        csv.header[1] == "col_2" &&
        csv.rows() == num_rows && csv.columns() == num_columns &&
        csv[0][0] == "1:\"somestring\"1" &&
        csv[0][1] == "2:1" &&
        csv[1][0] == "1:\"somestring\"2" &&
        csv[1][1] == "2:2") {
        t->fail = false;
    }
}

static void testCSVParserRecordWithFollowedQuotes(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "\"1:1\"    ,2:1\n";
    csv_string += "1:2,\"2:2\"\n";
    csv_string += (char)EOF; // 255?
    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<std::string> csv(io);
        csv.read(true);
        loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Quoted string cannot be followed by a non-delimiting character", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserRecordWithPrecededQuotes(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "    \"1:1\",2:1\n";
    csv_string += "1:2,\"2:2\"\n";
    csv_string += (char)EOF; // 255?

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    try {
        CSV<std::string> csv(io);
        csv.read(true);
        loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String:\n[\n%s]\n", csv.toString().c_str());
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        if (strcmp("Quoted string cannot be preceded by a non-delimiting character", e.what()) == 0) {
            t->fail = false;
        }
    }
}

static void testCSVParserFromRange(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "e:2,2:e\n";
    csv_string += (char)EOF; // 255?

    size_t num_rows = 9;
    size_t num_columns = 2;

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);
    t->fail = false;
    size_t range = num_rows;
    size_t dec = 2;
    while (range > 0) {
        size_t dec_actual = dec > range ? range: dec;
        csv.read(dec_actual);

        size_t remaining = range - dec_actual;
        loggerPrintf(LOGGER_TEST_VERBOSE, "CSV String [read %lu rows, %lu remaining (%lu range)]:\n[\n%s], actual rows in matrix: %lu\n", dec_actual, remaining, range, csv.toString().c_str(), csv.rows());
        loggerPrintf(LOGGER_TEST_VERBOSE, "FIRST ELEMENT: [%s, %s]\n", csv[0][0].c_str(), csv[0][1].c_str());
        if (true == (csv.rows() == dec_actual && csv.columns() == num_columns)) {
            if (range == 9 && (false == (csv[0][0] == "col_1" && csv[0][1] == "col_2"))) {
                t->fail = true;
                break;
            }
            if (range == 3 && (false == (csv[0][0] == "1:2" && csv[0][1] == "2:2"))) {
                t->fail = true;
                break;
            }
            if (range == 3 && (false == (csv[1][0] == "1:1" && csv[1][1] == "2:1"))) {
                t->fail = true;
                break;
            }
        // because obviously, we'll have more in buffer than actually read...
        } else if (range == 1 && (false == (csv[0][0] == "e:2" && csv[0][1] == "2:e"))) {
            t->fail = true;
            break;
        } else {
            t->fail = true;
            break;
        }
        range -= dec_actual;
    }
    if (csv.header.size() != 0) {
        t->fail = true;
    }
}

static void testCSVParserFromRangeOutOfRange(TestArg * t) {
    std::string csv_string("col_1,col_2\n");
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += "1:1,2:1\n";
    csv_string += "1:2,2:2\n";
    csv_string += (char)EOF; // 255?

    std::shared_ptr<EStream> io = std::make_shared<EStream>((uint8_t *)csv_string.data(), csv_string.size());
    CSV<std::string> csv(io);

    size_t range = 27;
    size_t dec = 2;
    try {
        while (range > 0) {
            size_t dec_actual = dec > range ? range: dec;
            csv.read(dec_actual);
            range -= dec_actual;
        }
    } catch(const std::runtime_error &e) {
        loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
        t->fail = false;
    }
}

// TODO: these might not be necessary lol because we aren't testing iostream.
static void testCSVParserFromFileAll(TestArg * t) {
    t->fail = false;
}
static void testCSVParserFromFileRange(TestArg * t) {
    t->fail = false;
}

int main(int argc, char * argv[]) {
    Tester t("CSV Parser Tests");

    t.addTest(testCSVParser);
    t.addTest(testCSVParserPeriodSeparator);
    t.addTest(testCSVParserDoubles);
    t.addTest(testCSVParserDoublesNonNumber);
    t.addTest(testCSVParserDoublesRecordWithInvalidNumberOfFields);
    t.addTest(testCSVParserSkipHeader);
    t.addTest(testCSVParserDelimeter);
    t.addTest(testCSVParserRecordWithNoFields);
    t.addTest(testCSVParserRecordWithInvalidNumberOfFields);
    t.addTest(testCSVParserRecordWithSpaces);
    t.addTest(testCSVParserRecordWithQuotes);
    t.addTest(testCSVParserRecordWithFollowedQuotes);
    t.addTest(testCSVParserRecordWithPrecededQuotes);
    t.addTest(testCSVParserRecordWithNestedQuotes);
    t.addTest(testCSVParserFromRange);
    t.addTest(testCSVParserFromRangeOutOfRange);
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