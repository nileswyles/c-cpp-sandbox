#include "tester.h"

#include "string-format.h"
#include "string_utils.h"

#include <iostream>
#include <sstream>

#ifndef LOGGER_STRING_FORMAT_TEST
#define LOGGER_STRING_FORMAT_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_STRING_FORMAT_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static void stringUtilsNumToString(TestArg * t) {
    std::string num = NumToString(-1234567, 10);
    std::string expected = "-1234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHex(TestArg * t) {
    std::string num = NumToString(-0x1234567, 16);
    std::string expected = "-0x1234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToString(TestArg * t) {
    // std::string num = FloatToString(1.1234567, 5);
    // lol... 
    std::string num = FloatToString(1.123, 7, -3);
    std::string expected = "1123.0000000E-3";
    ASSERT_STRING(t, num, expected);
    num = FloatToString(71.123, 7, -3);
    expected = "71123.0000000E-3";
    ASSERT_STRING(t, num, expected);
    num = FloatToString(1.123, 2, -3);
    expected = "1123.00E-3";
    ASSERT_STRING(t, num, expected);

    num = FloatToString(1.123, 5, 3);
    expected = "0.00112E+3";
    ASSERT_STRING(t, num, expected);
    num = FloatToString(1.123, 7, 3);
    expected = "0.0011230E+3";
    ASSERT_STRING(t, num, expected);
    num = FloatToString(7141.123, 7, 3);
    expected = "7.1411230000E+3";
    ASSERT_STRING(t, num, expected);

    num = FloatToString(7141.123, 1, 5);
    expected = "7.1411230000E+3";
    ASSERT_STRING(t, num, expected);
}

static void testFormat(TestArg * t) {
    std::string format = WylesLibs::format("Test Empty Format: (string) '{}'", "string1");
    std::string expected = "Test Empty Format: (string) 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatString(TestArg * t) {
    std::string format = WylesLibs::format("Test String Format: '{s}'", "string1");
    std::string expected = "Test String Format: 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatChar(TestArg * t) {
    std::string format = WylesLibs::format("Test Char Format: '{c}'", '$');
    std::string expected = "Test Char Format: '$'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatBool(TestArg * t) {
    std::string format = WylesLibs::format("Test Bool Format: '{b}'", true);
    std::string expected = "Test Bool Format: 'true'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDec(TestArg * t) {
    std::string format = WylesLibs::format("Test Bool Format: '{d}'", 77);
    std::string expected = "Test Dec Format: '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Hex Format: '{X}'", 255);
    std::string expected = "Test Hex Format: 'FF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDouble(TestArg * t) {
    std::string format = WylesLibs::format("Test Double Format: '{f}'", 1.1313131313131);
    std::string expected = "Test Double Format: '1.131313'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDoubleCustom(TestArg * t) {
    std::string format = WylesLibs::format("Test Double Format: '{03f}'", 1.1313131313131);
    std::string expected = "Test Double Format: '1.131'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatStream(TestArg * t) {
    std::stringstream ss;
    ss << "StringStream1";
    std::string format = WylesLibs::format("Test '{t}'", &ss);
    std::string expected = "Test Stream 'StringStream1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReference(TestArg * t) {
    std::string format = WylesLibs::format("Test '{}', '{1}'", "string1");
    std::string expected = "Test Reference 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverride(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{f}', '{1,03f}'", 1.13131313131313131313);
    std::string expected = "Test Reference template override '1.131313', '1.131'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatEscaping(TestArg * t) {
    std::cout << WylesLibs::format("\\{{}: {}\\}", "key", "value");
    // std::cout << WylesLibs::format("\{
    //                                     {}: {}
    //                                 \}", "key", "value"); ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
static void testFormatInvalidTypeForFormat(TestArg * t) {
    std::string format = WylesLibs::format("Test Invalid Type For Format: '{c}'", 77.000);
    std::string expected = "Test Invalid Type For Format: '$'";

    // these should through exceptions?

    ASSERT_STRING(t, format, expected);
}
static void testFormatInvalidTypeForFormatOverride(TestArg * t) {
    std::string format = WylesLibs::format("Test Invalid Type For Format Override: '{c}', '{1,x}'", '$');
    std::string expected = "Test Char Format: '$'";

    // these should through exceptions?

    ASSERT_STRING(t, format, expected);
}
static void testFormatInvalidIndicator(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Indicator: '{c}', '{7}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
    try {
        WylesLibs::format("Test Invalid Indicator: '{c}', '{ajknslnl}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
    try {
        WylesLibs::format("Test Invalid Indicator: '{c}', '{7kalnal}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
    try {
        WylesLibs::format("Test Invalid Indicator: '{c}', '{aakl7kalnal}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
    // ASSERT_STRING(t, format, expected);
}
static void testFormatUnimplementedFormat(TestArg * t) {

}
static void testFormatInvalidNotPercentStart(TestArg * t) {

}
static void testFormatInvalidIndicatorFormat(TestArg * t) {

}
static void testFormatInvalidPositionalFormat(TestArg * t) {

}

// TODO: do I care to require percent in format? it's not required for ease of parsing...

int main(int argc, char * argv[]) {
    Tester t("String Format Tests");

    t.addTest(stringUtilsNumToString);
    t.addTest(stringUtilsNumToStringHex);
    t.addTest(stringUtilsFloatToString);

    t.addTest(testFormat);
    t.addTest(testFormatString);
    t.addTest(testFormatChar);
    t.addTest(testFormatBool);
    t.addTest(testFormatDec);
    t.addTest(testFormatHex);
    t.addTest(testFormatDouble);
    t.addTest(testFormatDoubleCustom);
    t.addTest(testFormatStream);
    t.addTest(testFormatReference);
    t.addTest(testFormatReferenceTemplateOverride);
    t.addTest(testFormatEscaping);

    t.addTest(testFormatInvalidTypeForFormat);
    t.addTest(testFormatInvalidTypeForFormatOverride);

    t.addTest(testFormatInvalidIndicator);
    t.addTest(testFormatUnimplementedFormat);
    t.addTest(testFormatInvalidNotPercentStart);
    t.addTest(testFormatInvalidIndicatorFormat);
    t.addTest(testFormatInvalidPositionalFormat);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}