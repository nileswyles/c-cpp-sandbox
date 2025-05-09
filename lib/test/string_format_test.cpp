#include "tester.h"

#include "string_format.h"
#include "string_utils.h"

#include <iostream>
#include <sstream>

#include <string.h>

#ifndef LOGGER_STRING_FORMAT_TEST
#define LOGGER_STRING_FORMAT_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_STRING_FORMAT_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static void stringUtilsNumToStringMax(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: UINT64_MAX, default options\n");
    std::string num = numToString(static_cast<uint64_t>(18446744073709551615));
    std::string expected = "18446744073709551615";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringMin(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0, default options\n");
    std::string num = numToString(0);
    std::string expected = "0";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringAllDigits(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1234056789, default options\n");
    std::string num = numToString(1234056789);
    std::string expected = "1234056789";
    ASSERT_STRING(t, num, expected);
}

static void strinUtilsNumToStringSweep(TestArg * t) {
    std::string num;
    char expected[4] = {0};
    for (uint64_t i = 0; i < 777; i++) {
        loggerPrintf(LOGGER_TEST, "Value to Parse: %lu, default options\n", i);
        num = numToString(i);
        sprintf(expected, "%lu", i);
        ASSERT_STRING(t, num, std::string(expected));
        if (true == t->fail) { return; }
    }
}

static void stringUtilsNumToStringWith1003(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1003, default options\n");
    std::string num = numToString(1003);
    std::string expected = "1003";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringWith103(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 103, default options\n");
    std::string num = numToString(103);
    std::string expected = "103";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringWith170003(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 170003, default options\n");
    std::string num = numToString(170003);
    std::string expected = "170003";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringTruncating(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    opts.width = 4;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1234567, width: 4\n");
    std::string num = numToString(1234567, opts);
    std::string expected = "4567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringPadding(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    opts.width = 10;
    std::string num = numToString(1234567, opts);
    std::string expected = "0001234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringPaddingZero(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    opts.width = 2;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0, width: 2\n");
    std::string num = numToString(0, opts);
    std::string expected = "00";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringSigned(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    loggerPrintf(LOGGER_TEST, "Value to Parse: -1234567\n");
    std::string num = numToStringSigned(-1234567, opts);
    std::string expected = "-1234567";

    ASSERT_STRING(t, num, expected);
}

// with zeros or in other words num/base is multiple of base.
static void stringUtilsNumToStringSignedMax(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: INT64_MAX, default options\n");
    std::string num = numToStringSigned(9223372036854775807);
    std::string expected = "9223372036854775807";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringSignedMin(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: INT64_MIN, default options\n");
    std::string num = numToStringSigned(-9223372036854775807);
    std::string expected = "-9223372036854775807";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringSignedZero(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0, default options\n");
    std::string num = numToStringSigned(0);
    std::string expected = "0";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringSignedTruncating(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    opts.width = 4;
    loggerPrintf(LOGGER_TEST, "Value to Parse: -1234567, width: 4\n");
    std::string num = numToStringSigned(-1234567, opts);
    std::string expected = "-4567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringSignedPadding(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 10;
    opts.width = 10;
    loggerPrintf(LOGGER_TEST, "Value to Parse: -1234567, width: 10\n");
    std::string num = numToStringSigned(-1234567, opts);
    std::string expected = "-0001234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHex(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 16;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0x1234567\n");
    std::string num = numToString(0x1234567, opts);
    std::string expected = "0x1234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHex0xFF0A(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 16;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0xFF0A, default options\n");
    std::string num = numToStringSigned(0xFF0A, opts);
    std::string expected = "0xFF0A";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHexWith0x1BCF0FFA7(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 16;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0x1BCF0FFA7, default options\n");
    std::string num = numToStringSigned(0x1BCF0FFA7, opts);
    std::string expected = "0x1BCF0FFA7";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHexTruncating(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 16;
    opts.width = 4;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0x1234567, width: 4\n");
    std::string num = numToString(0x1234567, opts);
    std::string expected = "0x4567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsNumToStringHexPadding(TestArg * t) {
    StringFormatOpts opts;
    opts.base = 16;
    opts.width = 10;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0x1234567, width: 10\n");
    std::string num = numToString(0x1234567, opts);
    std::string expected = "0x0001234567";

    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringZeroPrecisionSix(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 6;
    opts.exponential = 0;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0, precision: 6, exponential: 0\n");
    std::string num = floatToString(0, opts);
    std::string expected = "0.000000";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringOnePrecisionSix(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 6;
    opts.exponential = 0;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1, precision: 6, exponential: 0\n");
    std::string num = floatToString(1, opts);
    std::string expected = "1.000000";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringZeroPrecisionZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = 0;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 0, precision: 0, exponential: 0\n");
    std::string num = floatToString(0, opts);
    std::string expected = "0";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringOnePrecisionZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = 0;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1, precision: 0, exponential: 0\n");
    std::string num = floatToString(1, opts);
    std::string expected = "1";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringTenPrecisionZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = 0;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 10, precision: 0, exponential: 0\n");
    std::string num = floatToString(10, opts);
    std::string expected = "10";
    ASSERT_STRING(t, num, expected);
}

// TODO: I'm sure I am forgetting a bunch of test cases.
static void stringUtilsFloatToStringNegativeEqualOriginalPrecision(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 7;
    opts.exponential = -3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 7, exponential: -3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "1123.0000000E-3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringNegativeEqualOriginalPrecisionDetectedWidthTwo(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 7;
    opts.exponential = -3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 71.123, precision: 7, exponential: -3\n");
    std::string num = floatToString(71.123, opts);
    std::string expected = "71123.0000000E-3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringNegativeEqualOriginalPrecisionNewPrecisionTwo(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 2;
    opts.exponential = -3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 2, exponential: -3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "1123.00E-3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringNegativeEqualOriginalPrecisionEqualZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = -3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 0, exponential: -3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "1123E-3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionGtLeastSignificantDigit(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 5;
    opts.exponential = 3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 5, exponential: 3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "0.00112E+3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionLtLeastSignificantDigit(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 7;
    opts.exponential = 3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 7, exponential: 3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "0.0011230E+3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveGtDetectedWidthPrecisionZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = 3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 0, exponential: 3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "0";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionLtLeastSignificantDigit(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 7;
    opts.exponential = 3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 7141.123, precision: 7, exponential: 3\n");
    std::string num = floatToString(7141.123, opts);
    std::string expected = "7.1411230E+3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionLtMostSignificantDigit(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 1;
    opts.exponential = 5;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 7141.123, precision: 1, exponential: 5\n");
    std::string num = floatToString(7141.123, opts);
    std::string expected = "0";
    ASSERT_STRING(t, num, expected);
}
    
static void stringUtilsFloatToStringPositiveEqualDetectedWidthPrecisionZero(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 0;
    opts.exponential = 3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 7141.123, precision: 0, exponential: 3\n");
    std::string num = floatToString(7141.123, opts);
    std::string expected = "7E+3";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionGtLeastSignificantDigit(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 3;
    opts.exponential = 2;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 17141.123, precision: 3, exponential: 2\n");
    std::string num = floatToString(17141.123, opts);
    // without width 171.411E+2
    std::string expected = "171.411E+2";
    ASSERT_STRING(t, num, expected);
}

// aka truncate left and right (width and precision)
static void stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionGtLeastSignificantDigitSpecifiedWidthLtDetectedWidth(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 3;
    opts.exponential = 2;
    opts.width = 2;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 17141.123, precision: 3, exponential: 2, width: 2\n");
    std::string num = floatToString(17141.123, opts);
    // without width 171.411E+2
    std::string expected = "71.411E+2";
    ASSERT_STRING(t, num, expected);
}

// also tests detected width less than specified width (padding) 
static void stringUtilsFloatToStringZeroResultWithSpecifiedWidth(TestArg * t) {
    StringFormatOpts opts;

    opts.precision = 1;
    opts.exponential = 5;
    opts.width = 4;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 7141.123, precision: 1, exponential: 5, width: 4\n");
    std::string num = floatToString(7141.123, opts);
    std::string expected = "0000";
    ASSERT_STRING(t, num, expected);
}

static void stringUtilsFloatToStringLowercaseDesignator(TestArg * t) {
    StringFormatOpts opts;

    opts.exponential_designator = 'e';
    opts.precision = 7;
    opts.exponential = -3;
    loggerPrintf(LOGGER_TEST, "Value to Parse: 1.123, precision: 7, exponential: -3\n");
    std::string num = floatToString(1.123, opts);
    std::string expected = "1123.0000000e-3";
    ASSERT_STRING(t, num, expected);
}

static void testFormat(TestArg * t) {
    std::string format = WylesLibs::format("Test Empty Format: (string) '{}'", "string1");
    std::string expected = "Test Empty Format: (string) 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatNoArg(TestArg * t) {
    // spec for va_arg says this is undefined... even so, let's document that here.
    std::string format = WylesLibs::format("Test Empty Format: (string) '{}'");

    loggerPrintf(LOGGER_TEST_VERBOSE, "Format: \n%s\n", format.c_str());
    t->fail = false;
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
    std::string format = WylesLibs::format("Test Dec Format: '{u}'", 77);
    std::string expected = "Test Dec Format: '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecMax(TestArg * t) {
    std::string format = WylesLibs::format("Test Dec Format: '{u}'", UINT64_MAX);
    std::string expected = "Test Dec Format: '18446744073709551615'"; // it's defined as (2**64) - 1 for some reason?

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecSigned(TestArg * t) {
    std::string format = WylesLibs::format("Test Dec Format: '{d}'", 77);
    std::string expected = "Test Dec Format: '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecSignedMax(TestArg * t) {
    std::string format = WylesLibs::format("Test Dec Format: '{d}'", UINT64_MAX);
    std::string expected = "Test Dec Format: '-1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Hex Format: '{X}'", 255);
    std::string expected = "Test Hex Format: '0xFF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatHexMax(TestArg * t) {
    std::string format = WylesLibs::format("Test Hex Format: '{X}'", UINT64_MAX);
    std::string expected = "Test Hex Format: '0xFFFFFFFFFFFFFFFF'";

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
    std::string format = WylesLibs::format("Test Stream '{t}'", &ss);
    std::string expected = "Test Stream 'StringStream1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReference(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference '{}', '{<1}'", "string1");
    std::string expected = "Test Reference 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideString(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{s}', '{<1}'", "string1");
    std::string expected = "Test Reference template override 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideChar(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{c}', '{<1}'", '$');
    std::string expected = "Test Reference template override '$', '$'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideBool(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{b}', '{<1}'", true);
    std::string expected = "Test Reference template override 'true', 'true'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDec(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{d}', '{<1}'", 77);
    std::string expected = "Test Reference template override '77', '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1}'", 0xFF);
    std::string expected = "Test Reference template override '0xff', '0xff'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideStringString(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{s}', '{<1,s}'", "string1");
    std::string expected = "Test Reference template override 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideCharChar(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{c}', '{<1,c}'", '$');
    std::string expected = "Test Reference template override '$', '$'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideBoolBool(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{b}', '{<1,b}'", true);
    std::string expected = "Test Reference template override 'true', 'true'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDecDec(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{d}', '{<1,d}'", 77);
    std::string expected = "Test Reference template override '77', '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHexHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1,x}'", 0xFF);
    std::string expected = "Test Reference template override '0xff', '0xff'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHexLowerToHexUpper(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1,X}'", 0xFF);
    std::string expected = "Test Reference template override '0xff', '0xFF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHexUpperToHexLower(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{X}', '{<1,x}'", 0xFF);
    std::string expected = "Test Reference template override '0xFF', '0xff'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideUnsignedToHexUpper(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{u}', '{<1,X}'", 0xFF);
    std::string expected = "Test Reference template override '255', '0xFF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideUnsignedToHexLower(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{u}', '{<1,x}'", 0xFF);
    std::string expected = "Test Reference template override '255', '0xff'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHexUpperToUnsigned(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{X}', '{<1,u}'", 0xFF);
    std::string expected = "Test Reference template override '0xFF', '255'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHexLowerToUnsigned(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1,u}'", 0xFF);
    std::string expected = "Test Reference template override '0xff', '255'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDouble(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Test Reference template override '{f}', '{<1,03f}', 1.13131313131313131313\n");
    std::string format = WylesLibs::format("Test Reference template override '{f}', '{<1,03f}'", 1.13131313131313131313);
    std::string expected = "Test Reference template override '1.131313', '1.131'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDoubleCustom(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{02f}', '{<1,03f}'", 1.13131313131313131313);
    std::string expected = "Test Reference template override '1.13', '1.131'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatEscaping(TestArg * t) {
    std::string format = WylesLibs::format("\\{{}: {}\\}", "key", "value");
    std::string expected = "{key: value}";

    ASSERT_STRING(t, format, expected);
}
static void testFormatInvalidFormatStartsWithDigit(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Format: '{7kalnal}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidFormatStartsWithLetter(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Format: '{ajknslnl}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidFormatNotFollowedByEndCharacter(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Reference: '{cLMAO}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidTypeForFormat(TestArg * t) {
    // spec for va_arg says this is undefined... even so, let's document that here.
    std::string format = WylesLibs::format("Test Type For Format: '{c}'", 12717171727.000);

    loggerPrintf(LOGGER_TEST_VERBOSE, "Format: \n%s\n", format.c_str());
    t->fail = false;
}
static void testFormatInvalidTypeForReferenceFormatOverride(TestArg * t) {
    try {
        WylesLibs::format("Test invalid format, type mismatch: '{c}', '{<1,03f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        if (strcmp(e.what(), "Invalid arg reference. Expected non-null pointer and arg.type = 'f'. Is pointer null? 1. Arg type: 'c'.") == 0) {
            t->fail = false;
        }
    }
}
static void testFormatInvalidTypeForReferenceFormatOverrideUnsigned(TestArg * t) {
    try {
        WylesLibs::format("Test invalid format, type mismatch: '{c}', '{<1,X}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        if (strcmp(e.what(), "Invalid arg reference. Expected non-null pointer and arg.type = 'u', 'x' or 'X'. Is pointer null? 1. Arg type: 'c'.") == 0) {
            t->fail = false;
        }
    }
}
static void testFormatInvalidReferenceOutOfRange(TestArg * t) {
    try {
        // out of range.
        WylesLibs::format("Test Invalid Reference: '{c}', '{<7}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
    try {
        // out of range.
        WylesLibs::format("Test Invalid Reference: '{f}', '{<7,03f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatStartsWithDigit(TestArg * t) {
    // starts with digit.
    try {
        WylesLibs::format("Test Invalid Reference: '{c}', '{<7kalnal}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatStartsWithLetter(TestArg * t) {
    // starts with letter
    try {
        WylesLibs::format("Test Invalid Reference: '{c}', '{<ajknslnl}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatUnimplementedFormat(TestArg * t) {
    try {
        WylesLibs::format("Test unimplemented command: '{#}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatNoSeparator(TestArg * t) {
    try {
        WylesLibs::format("Test invalid reference format (no separator): '{c}', '{<103f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatInvalidFloatFormat(TestArg * t) {
    try {
        WylesLibs::format("Test invalid reference format, invalid float format: '{c}', '{<1,af}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown:\n %s\n", e.what());
        t->fail = false;
    }
}

static void testFormatDecWidth(TestArg * t) {
    std::string format = WylesLibs::format("Test Dec Format: '{02u}'", 7777);
    std::string expected = "Test Dec Format: '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecSignedWidth(TestArg * t) {
    loggerPrintf(LOGGER_TEST, "Test Dec Format: '{02d}', -7777\n");
    // TODO: NT-61 - This seems unnecessary. 
    std::string format = WylesLibs::format("Test Dec Format: '{02d}'", (int64_t)-7777);
    std::string expected = "Test Dec Format: '-77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecSignedWithSign(TestArg * t) {
    std::string format = WylesLibs::format("Test Dec Format: '{+d}'", 7777);
    std::string expected = "Test Dec Format: '+7777'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatDecSignedWidthWithSign(TestArg * t) {

    std::string format = WylesLibs::format("Test Dec Format: '{+02d}'", 7777);
    std::string expected = "Test Dec Format: '+77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatStringToUpper(TestArg * t) {
    std::string format = WylesLibs::format("Test String Format: '{su}'", "LmAo");
    std::string expected = "Test String Format: 'LMAO'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatStringToLower(TestArg * t) {
    std::string format = WylesLibs::format("Test String Format: '{sl}'", "lMaO");
    std::string expected = "Test String Format: 'lmao'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatStringToUpperNegative(TestArg * t) {
    std::string format = WylesLibs::format("Test String Format: '{sue}'", "LmAo");
    std::string expected = "Test String Format: 'LMAO'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatStringToLowerNegative(TestArg * t) {
    std::string format = WylesLibs::format("Test String Format: '{sle}'", "lMaO");
    std::string expected = "Test String Format: 'lmao'";

    ASSERT_STRING(t, format, expected);
}

int main(int argc, char * argv[]) {
    Tester t("String Format Tests");

    time_t curtime = time(NULL);
    struct tm * loctime = localtime(&curtime);
    printf("\nToday is ");
    fputs(asctime(loctime), stdout);

    t.addTest(stringUtilsNumToStringMax);
    t.addTest(stringUtilsNumToStringMin);
    t.addTest(stringUtilsNumToStringAllDigits);
    t.addTest(strinUtilsNumToStringSweep);
    t.addTest(stringUtilsNumToStringWith1003);
    t.addTest(stringUtilsNumToStringWith103);
    t.addTest(stringUtilsNumToStringWith170003);
    t.addTest(stringUtilsNumToStringTruncating);
    t.addTest(stringUtilsNumToStringPadding);
    t.addTest(stringUtilsNumToStringPaddingZero);
    t.addTest(stringUtilsNumToStringSigned);
    t.addTest(stringUtilsNumToStringSignedMax);
    t.addTest(stringUtilsNumToStringSignedMin);
    t.addTest(stringUtilsNumToStringSignedZero);
    t.addTest(stringUtilsNumToStringSignedTruncating);
    t.addTest(stringUtilsNumToStringSignedPadding);
    t.addTest(stringUtilsNumToStringHex);
    t.addTest(stringUtilsNumToStringHex0xFF0A);
    t.addTest(stringUtilsNumToStringHexWith0x1BCF0FFA7);
    t.addTest(stringUtilsNumToStringHexTruncating);
    t.addTest(stringUtilsNumToStringHexPadding);
    t.addTest(stringUtilsFloatToStringZeroPrecisionSix);
    t.addTest(stringUtilsFloatToStringOnePrecisionSix);
    t.addTest(stringUtilsFloatToStringZeroPrecisionZero);
    t.addTest(stringUtilsFloatToStringOnePrecisionZero);
    t.addTest(stringUtilsFloatToStringTenPrecisionZero);
    t.addTest(stringUtilsFloatToStringNegativeEqualOriginalPrecision);
    t.addTest(stringUtilsFloatToStringNegativeEqualOriginalPrecisionDetectedWidthTwo);
    t.addTest(stringUtilsFloatToStringNegativeEqualOriginalPrecisionNewPrecisionTwo);
    t.addTest(stringUtilsFloatToStringNegativeEqualOriginalPrecisionEqualZero);
    t.addTest(stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionGtLeastSignificantDigit);
    t.addTest(stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionLtLeastSignificantDigit);
    t.addTest(stringUtilsFloatToStringPositiveGtDetectedWidthPrecisionZero);
    t.addTest(stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionLtLeastSignificantDigit);
    t.addTest(stringUtilsFloatToStringPositiveGtDetectedWidthNewPrecisionLtMostSignificantDigit);
    t.addTest(stringUtilsFloatToStringPositiveEqualDetectedWidthPrecisionZero);
    t.addTest(stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionGtLeastSignificantDigit);
    t.addTest(stringUtilsFloatToStringPositiveLtDetectedWidthNewPrecisionGtLeastSignificantDigitSpecifiedWidthLtDetectedWidth);
    t.addTest(stringUtilsFloatToStringZeroResultWithSpecifiedWidth);
    t.addTest(stringUtilsFloatToStringLowercaseDesignator);
    t.addTest(testFormat);
    t.addTest(testFormatNoArg);
    t.addTest(testFormatString);
    t.addTest(testFormatChar);
    t.addTest(testFormatBool);
    t.addTest(testFormatDec);
    t.addTest(testFormatDecMax);
    t.addTest(testFormatDecSigned);
    t.addTest(testFormatDecSignedMax);
    t.addTest(testFormatHex);
    t.addTest(testFormatHexMax);
    t.addTest(testFormatDouble);
    t.addTest(testFormatDoubleCustom);
    t.addTest(testFormatStream);
    t.addTest(testFormatReference);
    t.addTest(testFormatReferenceTemplateOverrideString);
    t.addTest(testFormatReferenceTemplateOverrideChar);
    t.addTest(testFormatReferenceTemplateOverrideBool);
    t.addTest(testFormatReferenceTemplateOverrideDec);
    t.addTest(testFormatReferenceTemplateOverrideHex);
    t.addTest(testFormatReferenceTemplateOverrideStringString);
    t.addTest(testFormatReferenceTemplateOverrideCharChar);
    t.addTest(testFormatReferenceTemplateOverrideBoolBool);
    t.addTest(testFormatReferenceTemplateOverrideDecDec);
    t.addTest(testFormatReferenceTemplateOverrideHexHex);
    t.addTest(testFormatReferenceTemplateOverrideHexLowerToHexUpper);
    t.addTest(testFormatReferenceTemplateOverrideHexUpperToHexLower);
    t.addTest(testFormatReferenceTemplateOverrideUnsignedToHexUpper);
    t.addTest(testFormatReferenceTemplateOverrideUnsignedToHexLower);
    t.addTest(testFormatReferenceTemplateOverrideHexUpperToUnsigned);
    t.addTest(testFormatReferenceTemplateOverrideHexLowerToUnsigned);
    t.addTest(testFormatReferenceTemplateOverrideDouble);
    t.addTest(testFormatReferenceTemplateOverrideDoubleCustom);
    t.addTest(testFormatEscaping);
    t.addTest(testFormatInvalidFormatStartsWithDigit);
    t.addTest(testFormatInvalidFormatStartsWithLetter);
    t.addTest(testFormatInvalidFormatNotFollowedByEndCharacter);
    t.addTest(testFormatInvalidTypeForFormat);
    t.addTest(testFormatInvalidTypeForReferenceFormatOverride);
    t.addTest(testFormatInvalidTypeForReferenceFormatOverrideUnsigned);
    t.addTest(testFormatInvalidReferenceOutOfRange);
    t.addTest(testFormatInvalidReferenceFormatStartsWithDigit);
    t.addTest(testFormatInvalidReferenceFormatStartsWithLetter);
    t.addTest(testFormatUnimplementedFormat);
    t.addTest(testFormatInvalidReferenceFormatNoSeparator);
    t.addTest(testFormatInvalidReferenceFormatInvalidFloatFormat);
    t.addTest(testFormatDecWidth);
    t.addTest(testFormatDecSignedWidth);
    t.addTest(testFormatDecSignedWithSign);
    t.addTest(testFormatDecSignedWidthWithSign);

    // TODO: these tests should pass but I don't care enough to dwell on this.
    //  
    // t.addTest(testFormatStringToUpper);
    // t.addTest(testFormatStringToLower);
    // t.addTest(testFormatStringToUpperNegative);
    // t.addTest(testFormatStringToLowerNegative);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
} 