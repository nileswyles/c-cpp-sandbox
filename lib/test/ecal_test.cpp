#include "tester.h"
#include "ecal.h"

#include <time.h>

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
    std::string expected = "Feb 27 2023, 00:00:00 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTime1970(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Jan 2 1970, 00:00:00 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLeapYearBeforeLeapDay(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Feb 27 2024, 00:00:00 Z";

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
    std::string expected = "Jan 1 2024, 00:00:00 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeLastDayOfLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Dec 31 2024, 23:59:59 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeFirstDayAfterLeapYear(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(0); // UTC
    std::string expected = "Jan 1 2025, 00:00:00 Z";

    ASSERT_STRING(t, time, expected);
}
static void testGetFormattedTimeMinus5(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(-500); // EST
    std::string expected = "Feb 27 2023, 00:00:00 -5:00";

    ASSERT_STRING(t, time, expected);
}

static void testGetFormattedTimeMinus5ISO8601_READABLE(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(-500, WylesLibs::Cal::ISO8601_READABLE); // EST
    std::string expected = "2023-02-27T00:00:00-5:00";

    ASSERT_STRING(t, time, expected);
}

static void testGetFormattedTimeMinus5ISO8601(TestArg * t) {
    std::string time = WylesLibs::Cal::getFormattedDateTime(-500, WylesLibs::Cal::ISO8601); // EST
    std::string expected = "20230227T000000-500";

    ASSERT_STRING(t, time, expected);
}

//  getEpoch from different types, different scenarios, with offset, etc...

static void testGetEpochFromFormattedDateTime(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Jan 1 1970, 00:00:00 Z");
    uint64_t expected = 0;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1Day(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Jan 2 1970, 00:00:00 Z");
    uint64_t expected = 86400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1DayMinusEST(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Jan 2 1970, 00:00:00 -5:00");
    uint64_t expected = 68400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeRandomDateTime(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("May 17 1997, 18:28:05 Z");
    uint64_t expected = 863893685;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDay(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Feb 27 2024, 00:00:00 Z"); // UTC
    uint64_t expected = 1708992000;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearAfterLeapDay(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Nov 17 2024, 14:57:24 Z"); // UTC
    uint64_t expected = 1731855444;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayBeforeLeapYear(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Dec 31 2023, 23:59:59 Z"); // UTC
    uint64_t expected = 1704067199;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayOfLeapYear(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Jan 1 2024, 00:00:00 Z"); // UTC
    uint64_t expected = 1704067200;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayOfLeapYear(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Dec 31 2024, 23:59:59 Z"); // UTC
    uint64_t expected = 1735689599;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayAfterLeapYear(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("Jan 1 2025, 00:00:00 Z"); // UTC
    uint64_t expected = 1735689600;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("1970-01-01T00:00:00Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 0;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1DayISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("1970-01-02T00:00:00Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 86400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1DayMinus500ISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("1970-01-02T00:00:00-5:00", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 68400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeRandomDateTimeISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("1997-05-17T18:28:05Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 863893685;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDayISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2024-02-27T00:00:00Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1708992000;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearAfterLeapDayISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2024-11-17T14:57:24Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1731855444;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayBeforeLeapYearISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2023-12-31T23:59:59Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1704067199;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayOfLeapYearISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2024-01-01T00:00:00Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1704067200;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayOfLeapYearISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2024-12-31T23:59:59Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1735689599;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayAfterLeapYearISO8601_READABLE(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("2025-01-01T00:00:00Z", WylesLibs::Cal::ISO8601_READABLE);
    uint64_t expected = 1735689600;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("19700101T000000Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 0;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1DayISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("19700102T000000Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 86400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimePlus1DayMinus500ISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("19700102T000000-500", WylesLibs::Cal::ISO8601);
    uint64_t expected = 68400;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeRandomDateTimeISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("19970517T182805Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 863893685;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDayISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20240227T000000Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1708992000;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLeapYearAfterLeapDayISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20241117T145724Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1731855444;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayBeforeLeapYearISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20231231T235959Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1704067199;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayOfLeapYearISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20240101T000000Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1704067200;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeLastDayOfLeapYearISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20241231T235959Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1735689599;

    ASSERT_UINT64(t, time, expected);
}

static void testGetEpochFromFormattedDateTimeFirstDayAfterLeapYearISO8601(TestArg * t) {
    uint64_t time = WylesLibs::Cal::getEpochFromFormattedDateTime("20250101T000000Z", WylesLibs::Cal::ISO8601);
    uint64_t expected = 1735689600;

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
    t.addTest(testGetFormattedTimeMinus5ISO8601_READABLE);
    t.addTest(testGetFormattedTimeMinus5ISO8601);
    t.addTest(testGetEpochFromFormattedDateTime);
    t.addTest(testGetEpochFromFormattedDateTimePlus1Day);
    t.addTest(testGetEpochFromFormattedDateTimePlus1DayMinusEST);
    t.addTest(testGetEpochFromFormattedDateTimeRandomDateTime);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDay);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearAfterLeapDay);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayBeforeLeapYear);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayOfLeapYear);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayOfLeapYear);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayAfterLeapYear);
    t.addTest(testGetEpochFromFormattedDateTimeISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimePlus1DayISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimePlus1DayMinus500ISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeRandomDateTimeISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDayISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearAfterLeapDayISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayBeforeLeapYearISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayOfLeapYearISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayOfLeapYearISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayAfterLeapYearISO8601_READABLE);
    t.addTest(testGetEpochFromFormattedDateTimeISO8601);
    t.addTest(testGetEpochFromFormattedDateTimePlus1DayISO8601);
    t.addTest(testGetEpochFromFormattedDateTimePlus1DayMinus500ISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeRandomDateTimeISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearBeforeLeapDayISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeLeapYearAfterLeapDayISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayBeforeLeapYearISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayOfLeapYearISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeLastDayOfLeapYearISO8601);
    t.addTest(testGetEpochFromFormattedDateTimeFirstDayAfterLeapYearISO8601);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}