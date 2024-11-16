#include "tester.h"

#include "string-format.h"
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
    printf("-------\n");
    num = FloatToString(71.123, 7, -3);
    expected = "71123.0000000E-3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(1.123, 2, -3);
    expected = "1123.00E-3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(1.123, 0, -3);
    expected = "1123.0E-3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(1.123, 5, 3);
    expected = "0.00112E+3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(1.123, 7, 3);
    expected = "0.0011230E+3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(1.123, 0, 3);
    expected = "0.0E+3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(7141.123, 7, 3);
    expected = "7.1411230000E+3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(7141.123, 1, 5);
    expected = "7.1411230000E+3";
    ASSERT_STRING(t, num, expected);
    printf("-------\n");
    num = FloatToString(7141.123, 0, 3);
    expected = "7.0E+3";
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
    std::string format = WylesLibs::format("Test Dec Format: '{d}'", 77);
    std::string expected = "Test Dec Format: '77'";

    ASSERT_STRING(t, format, expected);
}
// obviously depends on system but still. Curious what the limits are...
static void testFormatDecVaArgIntMax(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1,X}'", UINT64_MAX);
    std::string expected = "Test Reference template override '0xFFFFFFFFFFFFFFFF', '0xFFFFFFFFFFFFFFFF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Hex Format: '{X}'", 255);
    std::string expected = "Test Hex Format: '0xFF'";

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
    std::string format = WylesLibs::format("Test '{}', '{<1}'", "string1");
    std::string expected = "Test Reference 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideString(TestArg * t) {
    std::string format = WylesLibs::format("Test reference template override '{s}', '{<1,s}'", "string1");
    std::string expected = "Test Reference template override 'string1', 'string1'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideChar(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{c}', '{<1,c}'", '$');
    std::string expected = "Test Reference template override '$', '$'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideBool(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{b}', '{<1,b}'", true);
    std::string expected = "Test Reference template override 'true', 'true'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDec(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{d}', '{<1,d}'", 77);
    std::string expected = "Test Reference template override '77', '77'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideHex(TestArg * t) {
    std::string format = WylesLibs::format("Test Reference template override '{x}', '{<1,X}'", 0xFF);
    std::string expected = "Test Reference template override '0xFF', '0xFF'";

    ASSERT_STRING(t, format, expected);
}
static void testFormatReferenceTemplateOverrideDouble(TestArg * t) {
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
    // std::cout << "\\{" << std::endl;
    std::cout << WylesLibs::format("\\{{}: {}\\}", "key", "value");
    // \{key: value\} // after first iteration
    // {key: value} // after second iteration.
    // std::cout << WylesLibs::format("\{
    //                                     {}: {}
    //                                 \}", "key", "value"); ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
}
static void testFormatInvalidFormatStartsWithDigit(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Format: '{7kalnal}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidFormatStartsWithLetter(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Format: '{ajknslnl}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidFormatNotFollowedByEndCharacter(TestArg * t) {
    try {
        WylesLibs::format("Test Invalid Reference: '{cLMAO}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidTypeForFormat(TestArg * t) {
    try {
        std::string format = WylesLibs::format("Test Type For Format: '{c}'", 77.000);
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidTypeForReferenceFormatOverride(TestArg * t) {
    try {
        WylesLibs::format("Test invalid format, type mismatch: '{c}', '{<1,03f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        if (strcmp(e.what(), "Invalid arg selected. Expected non-null pointer and arg.type = 'f'. Is pointer null? 1. Arg type: '|'.") == 0) {
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
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
    try {
        // out of range.
        WylesLibs::format("Test Invalid Reference: '{f}', '{<7,03f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
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
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
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
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatUnimplementedFormat(TestArg * t) {
    try {
        WylesLibs::format("Test unimplemented command: '{#}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatNoSeparator(TestArg * t) {
    try {
        WylesLibs::format("Test invalid reference format (no separator): '{c}', '{<103f}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}
static void testFormatInvalidReferenceFormatInvalidFloatFormat(TestArg * t) {
    try {
        WylesLibs::format("Test invalid reference format, invalid float format: '{c}', '{<1,af}'", '$');
        t->fail = true;
        return;
    } catch (std::exception& e) {
        loggerPrintf(LOGGER_TEST, "Exception thrown: %s\n", e.what());
        t->fail = false;
    }
}

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
    t.addTest(testFormatDecVaArgIntMax);
    t.addTest(testFormatHex);
    t.addTest(testFormatDouble);
    t.addTest(testFormatDoubleCustom);
    t.addTest(testFormatStream);
    t.addTest(testFormatReference);
    t.addTest(testFormatReferenceTemplateOverrideString);
    t.addTest(testFormatReferenceTemplateOverrideChar);
    t.addTest(testFormatReferenceTemplateOverrideBool);
    t.addTest(testFormatReferenceTemplateOverrideDec);
    t.addTest(testFormatReferenceTemplateOverrideHex);
    t.addTest(testFormatReferenceTemplateOverrideDouble);
    t.addTest(testFormatReferenceTemplateOverrideDoubleCustom);
    t.addTest(testFormatEscaping);
    t.addTest(testFormatInvalidFormatStartsWithDigit);
    t.addTest(testFormatInvalidFormatStartsWithLetter);
    t.addTest(testFormatInvalidFormatNotFollowedByEndCharacter);
    t.addTest(testFormatInvalidTypeForFormat);
    t.addTest(testFormatInvalidTypeForReferenceFormatOverride);
    t.addTest(testFormatInvalidReferenceOutOfRange);
    t.addTest(testFormatInvalidReferenceFormatStartsWithDigit);
    t.addTest(testFormatInvalidReferenceFormatStartsWithLetter);
    t.addTest(testFormatUnimplementedFormat);
    t.addTest(testFormatInvalidReferenceFormatNoSeparator);
    t.addTest(testFormatInvalidReferenceFormatInvalidFloatFormat);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}