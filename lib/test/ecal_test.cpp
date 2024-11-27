#include "tester.h"
#include "ecal.h"

#ifndef LOGGER_ECAL_TEST
#define LOGGER_ECAL_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ECAL_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static void testGetFormattedTime(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Feb 27 2023, 0:0:0 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTime1970(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Jan 2 1970, 0:0:0 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLeapYearBeforeLeapDay(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Feb 27 2024, 0:0:0 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLeapYearAfterLeapDay(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Nov 17 2024, 14:57:24 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLastDayBeforeLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Dec 31 2023, 23:59:59 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeFirstDayOfLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Jan 1 2024, 0:0:0 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLastDayOfLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Dec 31 2024, 23:59:59 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeFirstDayAfterLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Jan 1 2025, 0:0:0 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeMinus5(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(-500); // EST
    std::string expected = "Feb 27 2023, 0:0:0 -5:00";

    ASSERT_STRING(t, time, expected);
}

static void testGetEpochFromFormattedDateTime(TestArg * t) {
    uint64_t time = getEpochFromFormattedDateTime("Jan 1 1970, 0:0:0 Z");
    uint64_t expected = 0;

    ASSERT_UINT64(t, time, expected);

    time = getEpochFromFormattedDateTime("Jan 2 1970, 0:0:0 Z");
    expected = 86400;

    ASSERT_UINT64(t, time, expected);

    epoch_time = getEpochFromFormattedDateTime("May 17 1997, 18:28:5 Z");
    expected = 863893685;

    ASSERT_UINT64(t, time, expected);
}

int main(int argc, char * argv[]) {
    Tester t("Calendar/Date/Time Tests");

    time_t curtime = time(NULL);
    struct tm * loctime = localtime(&curtime);
    printf("\nToday is ");
    fputs(asctime(loctime), stdout);

    t.addTest(testGetFormattedTime);
    t.addTest(testGetFormattedTime1970);
    t.addTest(testGetFormattedTimeLeapYearBeforeLeapDay);
    t.addTest(testGetFormattedTimeLeapYearAfterLeapDay);
    t.addTest(testGetFormattedTimeLastDayBeforeLeapYear);
    t.addTest(testGetFormattedTimeFirstDayOfLeapYear);
    t.addTest(testGetFormattedTimeLastDayOfLeapYear);
    t.addTest(testGetFormattedTimeFirstDayAfterLeapYear);
    t.addTest(testGetFormattedTimeMinus5);
    t.addTest(testGetEpochFromFormattedDateTime);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}