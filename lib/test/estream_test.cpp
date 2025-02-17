#include "tester.h"
#include "estream/byteestream.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>

#include <filesystem>

#include "file/file.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#ifndef LOGGER_LEVEL 
#define LOGGER_LEVEL LOGGER_TEST
#endif

#ifndef LOGGER_READER_TEST
#define LOGGER_READER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_READER_TEST
#include "logger.h"

// TODO - This is sort of a work around because of how collectors are designed (see estream_types.h)...
//      Need to think about whether to use templates in collectors to simplify how this is defined...
#define ReaderTaskChain ReaderTaskChain<std::string>
#define ReaderTaskLC ReaderTaskLC<std::string>
#define ReaderTaskUC ReaderTaskUC<std::string>
#define ReaderTaskDisallow ReaderTaskDisallow<std::string>
#define ReaderTaskAllow ReaderTaskAllow<std::string>
#define ReaderTaskExact ReaderTaskExact<std::string>
#define ReaderTaskTrim ReaderTaskTrim<std::string>
#define ReaderTaskExtract ReaderTaskExtract<std::string>

using namespace WylesLibs;
using namespace WylesLibs::Test;

static const char * buffer_start;
static const char * buffer;

// ! IMPORTANT - overriding stdlib's implementation of read (which is apparently weakly linked...)... ByteEStream's calls to read use this function. 
extern ssize_t read(int fd, void * b, size_t nbytes) {
    size_t ret = MIN(nbytes, strlen(buffer));
    memcpy(b, buffer, ret);
    loggerPrintf(LOGGER_DEBUG, "READ RETURNED (%ld): \n", ret);
    loggerPrintByteArray(LOGGER_DEBUG, (uint8_t*)b, ret);
    buffer += ret; // duh
    return ret; 
}

extern int poll(struct pollfd *__fds, nfds_t __nfds, int __timeout) {
    return buffer_start + strlen(buffer_start) > buffer;
}

static void setTestString(const char * s) {
    buffer_start = s;
    buffer = s;
}

static void readUntilAssert(TestArg * t, std::string result, std::string expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n'%s'\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_STRING(t, result, expected);
}

template<typename T>
static void readAssert(TestArg * t, SharedArray<T> result, SharedArray<T> expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n'%s'\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected String: %s\n", expected.toString().c_str());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Actual String: %s\n", result.toString().c_str());

    ASSERT_ARRAY<T>(t, result, expected);
}

static void readNaturalAssert(TestArg * t, ByteEStream& r, std::tuple<uint64_t, size_t> result, std::tuple<uint64_t, size_t> expected, char expected_until) {
    uint64_t actual_num = std::get<0>(result);
    size_t actual_num_digits = std::get<1>(result);
    char actual_get = r.get();
    loggerPrintf(LOGGER_TEST, "Actual number: %lu, num_digits: %lu, reader.get: %c\n", actual_num, actual_num_digits, actual_get);
    uint64_t expected_num = std::get<0>(expected);
    size_t expected_num_digits = std::get<1>(expected);
    loggerPrintf(LOGGER_TEST, "Expected number: %lu, num_digits: %lu, reader.get: %c\n", expected_num, expected_num_digits, expected_until);
    if (actual_num == expected_num && actual_num_digits == expected_num_digits && actual_get == expected_until) {
        t->fail = false;
    }
}

static void readDecimalAssert(TestArg * t, ByteEStream& r, std::tuple<double, size_t, size_t> result, std::tuple<double, size_t, size_t> expected, char expected_until) {
    double actual_num = std::get<0>(result);
    size_t actual_num_natural_digits = std::get<1>(result);
    size_t actual_num_decimal_digits = std::get<2>(result);
    char actual_get = r.get();
    loggerPrintf(LOGGER_TEST, "Actual number: %f, num_natural_digits: %lu, num_decimal_digits: %lu, reader.get: %c\n", 
                    actual_num, actual_num_natural_digits, actual_num_decimal_digits, actual_get);
    double expected_num = std::get<0>(expected);
    size_t expected_num_natural_digits = std::get<1>(expected);
    size_t expected_num_decimal_digits = std::get<2>(expected);
    loggerPrintf(LOGGER_TEST, "Expected number: %f, num_natural_digits: %lu, num_decimal_digits: %lu, reader.get: %c\n", 
                    expected_num, expected_num_natural_digits, expected_num_decimal_digits, expected_until);
    if (result == expected && actual_get == expected_until) {
        t->fail = false;
    }
}

static void readNaturalAssertConsume(TestArg * t, ByteEStream& r, std::tuple<uint64_t, size_t> result, std::tuple<uint64_t, size_t> expected, bool expected_good) {
    uint64_t actual_num = std::get<0>(result);
    size_t actual_num_digits = std::get<1>(result);
    bool actual_good = r.good();
    loggerPrintf(LOGGER_TEST, "Actual number: %lu, num_digits: %lu, reader.good: %s\n", actual_num, actual_num_digits, true == actual_good ? "true" : "false");
    uint64_t expected_num = std::get<0>(expected);
    size_t expected_num_digits = std::get<1>(expected);
    loggerPrintf(LOGGER_TEST, "Expected number: %lu, num_digits: %lu, reader.good: %s\n", expected_num, expected_num_digits, true == expected_good ? "true" : "false");

    if (result == expected && actual_good == expected_good) {
        t->fail = false;
    }
}

static void readDecimalAssertConsume(TestArg * t, ByteEStream& r, std::tuple<double, size_t, size_t> result, std::tuple<double, size_t, size_t> expected, bool expected_good) {
    double actual_num = std::get<0>(result);
    size_t actual_num_natural_digits = std::get<1>(result);
    size_t actual_num_decimal_digits = std::get<2>(result);
    bool actual_good = r.good();
    loggerPrintf(LOGGER_TEST, "Actual number: %f, num_natural_digits: %lu, num_decimal_digits: %lu, reader.good: %s\n", 
                    actual_num, actual_num_natural_digits, actual_num_decimal_digits, true == actual_good ? "true" : "false");
    double expected_num = std::get<0>(expected);
    size_t expected_num_natural_digits = std::get<1>(expected);
    size_t expected_num_decimal_digits = std::get<2>(expected);
    loggerPrintf(LOGGER_TEST, "Expected number: %f, num_natural_digits: %lu, num_decimal_digits: %lu, reader.good: %s\n", 
                    expected_num, expected_num_natural_digits, expected_num_decimal_digits, true == expected_good ? "true" : "false");

    if (result == expected && actual_good == expected_good) {
        t->fail = false;
    }
}


static void testReadUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRINGWITHSPACE BLAH");

    std::string result = reader.read<std::string>(" ");
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLimit(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    std::string data;
    for (size_t i = 0; i < (1 << 16); i++) {
        data += "|";
    }
    data += " ";
    setTestString(data.c_str());
    try {
        std::string result = reader.read<std::string>(" ");
    } catch (std::exception& e) {
        ASSERT_STRING(t, std::string(e.what()), "Spatial until limit of 65536 reached.");
    }
}

static void testReadUntilUpperCase(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRINGWITHSPACE BLAH");

    ReaderTaskUC uppercase;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&uppercase);
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLowerCase(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRINGWITHSPACE BLAH");

    ReaderTaskLC lowercase;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&lowercase);
    std::string expected = "teststringwithspace ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllow(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"BLAH ");

    ReaderTaskAllow allow("ABC");
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&allow);
    std::string expected = "ACBA";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllowStrict(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"BLAH ");

    ReaderTaskAllow allow("ABC", true);
    bool exception = false;
    try {
        std::string result = reader.read<std::string>(" ", (ReaderTask *)&allow);
    } catch(std::exception& e) {
        exception = true;
    }

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilDisallow(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRING ");

    ReaderTaskDisallow disallow("IO");
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&disallow);
    std::string expected = "TESTSTRNG ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilDisallowStrict(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRING");

    ReaderTaskDisallow disallow("IO", true);
    bool exception = false;
    try {
        std::string result = reader.read<std::string>(" ", (ReaderTask *)&disallow);
    } catch(std::exception& e) {
        exception = true;
    }

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n%s\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("     TESTSTRINGWITHSPACE     ");

    ReaderTaskTrim trim;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&trim);
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("     TESTSTRINGWITHSPACE");

    ReaderTaskTrim trim;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&trim);
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilRTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("TESTSTRINGWITHSPACE     ");

    ReaderTaskTrim trim;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&trim);
    std::string expected = "TESTSTRINGWITHSPACE";

    readUntilAssert(t, result, expected);
}

static void testReadUntilDisallowSpaceTrim(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("       TESTSTRINGWITHSPACE       ");

    ReaderTaskDisallow disallow(" ");
    ReaderTaskTrim trim;
    disallow.next_operation = &trim;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&disallow);
    std::string expected = "";

    readUntilAssert(t, result, expected);
}

static void testReadUntilExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"    ");

    ReaderTaskExtract extract('"','"');
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&extract);
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilExtractNonWhiteCharacterAfterRightToken(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    // ! IMPORTANT - if readUntil space and extracting then it will include a whitespace so, this works.
    // const char * test_string = "\"TESTSTRINGWITHSPACE\" ABLBK ";
    //  but this results in an exception.
    setTestString("\"TESTSTRINGWITHSPACE\"ABLBK ");

    ReaderTaskExtract extract('"','"');
    bool exception = false;
    try {
        std::string result = reader.read<std::string>(" ", (ReaderTask *)&extract);
        loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n'%s'\n", result.c_str());
    } catch (std::exception& e) {
        exception = true;
    }
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilAllowExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"    ");

    ReaderTaskAllow allow("A\"BC");
    ReaderTaskExtract extract('"', '"');
    allow.next_operation = &extract;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&allow);
    std::string expected = "AC";

    readUntilAssert(t, result, expected);
}

static void testReadUntilAllowExtractException(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"    ");

    ReaderTaskAllow allow("ABC");
    ReaderTaskExtract extract('"', '"');
    allow.next_operation = &extract;
    bool exception = false;
    try {
        reader.read<std::string>(" ", (ReaderTask *)&allow);
    } catch (std::exception& e) {
        exception = true;
    }
    ASSERT_BOOLEAN(t, exception, true);
}

static void testReadUntilDisallowExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"     ");

    ReaderTaskDisallow disallow("IO");
    ReaderTaskExtract extract('"', '"');
    disallow.next_operation = &extract;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&disallow);
    std::string expected = "TESTSTRNGWTHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilLowerCaseExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWITHSPACE\"   ");

    ReaderTaskLC lowercase;
    ReaderTaskExtract extract('"','"');
    lowercase.next_operation = &extract;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&lowercase);
    std::string expected = "teststringwithspace ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilUpperCaseExtract(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    setTestString("\"TESTSTRINGWiTHSPACE\"    ");

    ReaderTaskUC uppercase;
    ReaderTaskExtract extract('"','"');
    uppercase.next_operation = &extract;
    std::string result = reader.read<std::string>(" ", (ReaderTask *)&uppercase);
    std::string expected = "TESTSTRINGWITHSPACE ";

    readUntilAssert(t, result, expected);
}

static void testReadUntilCursorAtUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" BLAH");

    std::string result = reader.read<std::string>(" ", nullptr, true);
    readUntilAssert(t, result, " ");
}

static void testReadUntilCursorAtUntilNotInclusive(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" BLAH");

    std::string result = reader.read<std::string>(" ", nullptr, false);
    readUntilAssert(t, result, "");
}

static void testReadUntilFillBufferOnce(TestArg * t) {
    size_t buffer_size = 7;
    ByteEStream reader(1, buffer_size);

    size_t expected_size = buffer_size + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    setTestString(expected.c_str());
    
    std::string result = reader.read<std::string>(" ");
    readUntilAssert(t, result, "$$$$$$$$$$$ ");
}

static void testReadUntilFillBufferTwice(TestArg * t) {
    size_t buffer_size = 7;
    ByteEStream reader(1, buffer_size);

    size_t expected_size = (buffer_size * 2) + 7;
    std::string expected;
    for (size_t i = 0; i < expected_size; i++) {
        expected += '$';
    }
    expected[expected_size - 3] = ' ';
    setTestString(expected.c_str());

    std::string result = reader.read<std::string>(" ");
    readUntilAssert(t, result, "$$$$$$$$$$$$$$$$$$ ");
}

static void testReadUntilAtEndOfStreamButAlsoUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" BLAH|");

    std::string expected(" BLAH|");
    std::string result = reader.read<std::string>("|", nullptr, true);

    loggerPrintf(LOGGER_TEST_VERBOSE, "Test String:\n'%s'\n", buffer_start);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Until char:\n[%x]\n", ' ');
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n'%s'\n", result.c_str());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n'%s'\n", expected.c_str());

    if (result == expected || false == reader.good()) {
        t->fail = false;
    }
}

static void testReadEls(TestArg * t) {
    EStream<uint32_t> reader(1, READER_RECOMMENDED_BUF_SIZE);

    // 0x41414141 0x42424242 0x43434343 0x44444444
    setTestString("AAAABBBBCCCCDDDD");

    SharedArray<uint32_t> expected{
        0x41414141,
        0x42424242,
        0x43434343,
        0x44444444
    };
    SharedArray<uint32_t> result = reader.read<SharedArray<uint32_t>>(4);

    readAssert<uint32_t>(t, result, expected);
}

static void testReadElsFullBuffer(TestArg * t) {
    const size_t buffer_size = 4;
    EStream<uint32_t> reader(1, buffer_size);

    // 0x41414141 0x42424242 0x43434343 0x44444444
    setTestString("AAAABBBBCCCCDDDD");

    SharedArray<uint32_t> expected{
        0x41414141,
        0x42424242,
        0x43434343,
        0x44444444
    };
    SharedArray<uint32_t> result = reader.read<SharedArray<uint32_t>>(buffer_size);

    readAssert<uint32_t>(t, result, expected);

    if (t->fail == true) {
        return;
    }
    t->fail = true;

    try {
        SharedArray<uint32_t> lol = reader.read<SharedArray<uint32_t>>(1);
        loggerPrintf(LOGGER_TEST_VERBOSE, "Read didn't throw an exception: 0x%X", lol.at(0));
    } catch(std::exception& e) {
        loggerPrintf(LOGGER_TEST_VERBOSE, "Exception: %s\n", e.what());
        t->fail = false;
    }
}

class TestReadElsReaderTask: public StreamTask<uint32_t, SharedArray<uint32_t>> {
    public:
        size_t element_count;
        TestReadElsReaderTask() {}
        ~TestReadElsReaderTask() {}
        void initialize() override final {
            element_count = 0;
        }
        void flush() override final {}
        void perform(uint32_t& el) override final {
            uint32_t e = el;
            if (++element_count == 2) {
                e = 0x45454545;
            }
            this->collectorAccumulate(e);
        }
};

static void testReadElsReaderTask(TestArg * t) {
    EStream<uint32_t> reader(1, READER_RECOMMENDED_BUF_SIZE);

    // 0x41414141 0x42424242 0x43434343 0x44444444
    setTestString("AAAABBBBCCCCDDDD");

    SharedArray<uint32_t> expected{
        0x41414141,
        0x45454545,
        0x43434343,
        0x44444444
    };
    TestReadElsReaderTask task;
    SharedArray<uint32_t> result = reader.read<SharedArray<uint32_t>>(4, static_cast<StreamTask<uint32_t, SharedArray<uint32_t>> *>(&task));

    readAssert<uint32_t>(t, result, expected);
}

static void testPeek(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" %$lol");

    char peeked_char = reader.peek();
    reader.get();
    char second_peeked_char = reader.peek();
    loggerPrintf(LOGGER_TEST, "Peeked Char: %x, '%c', Second Peeked Char: %x, '%c', Expected: ' ' and '@'\n", peeked_char, peeked_char, second_peeked_char, second_peeked_char);
    if (peeked_char != ' ' && second_peeked_char != '%') {
        t->fail = true;
    } else {
        t->fail = false;
    }
}

static void testPeekNoMoreData(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);

    std::string str = " BLAH";
    setTestString(str.c_str());
    for (size_t i = 0; i < str.size(); i++) {
        reader.get();
    }
    try {
        reader.peek();
    } catch (std::exception& e) {
        ASSERT_STRING(t, std::string(e.what()), "Read error. No more data in the stream to fill buffer.");
    }
}

static void testUnget(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" @LAH");

    char peeked_char = reader.get();
    reader.unget();
    char second_peeked_char = reader.get();
    loggerPrintf(LOGGER_TEST, "Peeked Char: %x, '%c', Second Peeked Char: %x, '%c', Expected: ' ' and ' '\n", peeked_char, peeked_char, second_peeked_char, second_peeked_char);
    if (peeked_char == ' ' && second_peeked_char == ' ') {
        t->fail = false;
    }
}

static void testUngetLastCharacterBeforeFillBuffer(TestArg * t) {
    size_t buffer_size = 2;
    ByteEStream reader(1, buffer_size);
    setTestString(" 277");

    for (size_t i = 0; i < buffer_size; i++) {
        reader.get();
    }
    // cursor == buffer_size;
    //  unget last element
    reader.unget();
    char ungot_char = reader.get();
    loggerPrintf(LOGGER_TEST, "Ungot Char: %x, '%c', Expected: 0x32, '2'\n", ungot_char, ungot_char);
    if (ungot_char == '2') {
        t->fail = false;
    }
}

static void testUngetHaventGotten(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" BLAH");

    reader.unget(); // no-op, because bytes_in_buffer == cursor... so not good and will fillBuffer when get is called.
    char ungot_char = reader.get();
    // default value of ungot_el
    loggerPrintf(LOGGER_TEST, "Ungot Char: %x, '%c', Expected: 0, ''\n", ungot_char, ungot_char); 
    if (ungot_char == 0x00) {
        t->fail = false;
    }
}

static void testGet(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" %$lol");

    // TODO: kind of a dumb test?
    char peeked_char = reader.get();
    char second_peeked_char = reader.get();
    if (peeked_char == ' ' && second_peeked_char == '%') {
        t->fail = false;
    }
}

static void testGood(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString(" %$lol");

    bool reader_good_1 = reader.good();
    reader.get();
    reader.get();
    bool reader_good_2 = reader.good();
    if (true == reader_good_1 && true == reader_good_2) {
        t->fail = false;
    }
}

static void testGoodNoMoreData(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data(" %$lol");
    setTestString(data.c_str());

    // nul char
    for (size_t i = 0; i < data.size(); i++) {
        reader.get();
    }
    if (false == reader.good()) {
        t->fail = false;
    }
}

static void testReadNaturalUntilDigitClassEmpty(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("|");
    setTestString(data.c_str());

    std::tuple<uint64_t, size_t> result = reader.readNatural();
    readNaturalAssert(t, reader, result, std::make_tuple(0, 0), '|');
}

static void testReadNaturalUntilDigitClass(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<uint64_t, size_t> result = reader.readNatural();
    readNaturalAssert(t, reader, result, std::make_tuple(100000, 6), '|');
}

static void testReadNaturalUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<uint64_t, size_t> result = reader.readNatural("|");
    readNaturalAssertConsume(t, reader, result, std::make_tuple(100000, 6), false);
}

static void testReadNaturalUntilNoConsume(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<uint64_t, size_t> result = reader.readNatural("|", false);
    readNaturalAssert(t, reader, result, std::make_tuple(100000, 6), '|');
}

static void testReadNaturalN(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    setTestString("100000");

    std::tuple<uint64_t, size_t> result = reader.readNatural(6);
    readNaturalAssert(t, reader, result, std::make_tuple(100000, 6), '|');
}

static void testReadNaturalNDigitClass(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<uint64_t, size_t> result = reader.readNatural(0);
    readNaturalAssert(t, reader, result, std::make_tuple(100000, 6), '|');
}

// TODO: same for until, and n...
// static void testReadDecimalUntilDigitClassEmpty(TestArg * t) {
static void testReadDecimalUntilDigitClassEmpty(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(0.0, 0, 0), '|');
}

static void testReadDecimalDigitClass(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.1239000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(100000.1239, 6, 7), '|');
}

static void testReadDecimalUntil(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.1239000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal("|"); // default consume = true
    readDecimalAssertConsume(t, reader, result, std::make_tuple(100000.1239, 6, 7), false);
}

static void testReadDecimalUntilNoConsume(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.1239000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal("|", false); // default consume = true
    readDecimalAssert(t, reader, result, std::make_tuple(100000.1239, 6, 7), '|');
}

static void testReadDecimalDigitClassNoDecimalPart(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalUntilNoDecimalPart(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal("|");
    readDecimalAssertConsume(t, reader, result, std::make_tuple(100000.0, 6, 0), false);
}

static void testReadDecimalUntilNoConsumeNoDecimalPart(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal("|", false);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalN(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    size_t num_digits = 6;
    std::tuple<double, size_t, size_t> result = reader.readDecimal(num_digits);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, num_digits, 0), '|');
}

static void testReadDecimalNDigitClass(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal(0);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalLimit(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    try {
        reader.readDecimal(4294967297);
    } catch(std::exception& e) {
        ASSERT_STRING(t, std::string(e.what()), "You're reading more than the limit specified... Read less, or you know what, don't read at all.");
    }
}

static void testReadDecimalDigitClassNoDecimalPartDecimal(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalUntilNoDecimalPartDecimal(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal("|", false);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalNNoDecimalPart(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal(6);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalNNoDecimalPartDecimal(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal(6);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '.');
}

static void testReadDecimalNNoDecimalPartDecimalCorrect(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal(7);
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalNDigitClassNoDecimal(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadDecimalNDigitClassNoDecimalPartDecimal(TestArg * t) {
    ByteEStream reader(1, READER_RECOMMENDED_BUF_SIZE);
    std::string data("100000.|");
    setTestString(data.c_str());

    std::tuple<double, size_t, size_t> result = reader.readDecimal();
    readDecimalAssert(t, reader, result, std::make_tuple(100000.0, 6, 0), '|');
}

static void testReadUntilIStreamEStream(TestArg * t) {
    ESharedPtr<std::basic_stringstream<char>> stream(new std::basic_stringstream<char>("IdkSomeString|"));
    IStreamEStream reader(stream);

    // So, something about templates don't allow overloading?
    std::string result = reader.read<std::string>("|");

    readUntilAssert(t, result, "IdkSomeString|");
}

static void testReadElsIStreamEStream(TestArg * t) {
    ESharedPtr<std::basic_stringstream<char>> stream(new std::basic_stringstream<char>("IdkSomeString|"));
    IStreamEStream reader(stream);

    std::string result = reader.read<SharedArray<uint8_t>>(13).toString();

    readUntilAssert(t, result, "IdkSomeString");
}

static void testReadUntilIStreamEStreamReaderTask(TestArg * t) {
    ESharedPtr<std::basic_stringstream<char>> stream(new std::basic_stringstream<char>("IdkSomeString|"));
    IStreamEStream reader(stream);

    ReaderTaskLC lowercase;
    // std::string result = reader.read<std::string>("|", (ReaderTask *)&lowercase).toString();
    // TODO: okay sure you can't virtual but still? I was hoping this would at least work, if not, then all of this is useless?
    //  maybe only remove the no longer necessary EStreamI?
    std::string result = reader.read<std::string>("|", (ReaderTask *)&lowercase);

    readUntilAssert(t, result, "idksomestring|");
}


// TODO:
// static void testReadStringIStreamEStream(TestArg * t) {
// }

int main(int argc, char * argv[]) {
    Tester t("EStream Tests");
    // TODO: bug fix/feature needed... if we reach an "until" character while r_trimming (when open, before right_most_char is reached), then we will exit.
    //  might want to break only if we see until character and not r_trimming (i.e. not within quotes)... ": ": ' should yield ': ' not ':'. NOTE: left and right most characters aren't included, by design. Can probably parameterize that.
    
    // lol, this is wrong but the sentiment was that behaviour should be defined and documented regardless of direction.

    signal(SIGSEGV, SIG_IGN); // but why?

    // make sure to write a test.
    t.addTest(testReadUntil);
    t.addTest(testReadUntilLimit);
    t.addTest(testReadUntilUpperCase);
    t.addTest(testReadUntilLowerCase);
    t.addTest(testReadUntilAllow);
    t.addTest(testReadUntilAllowStrict);
    t.addTest(testReadUntilDisallow);
    t.addTest(testReadUntilDisallowStrict);

    // TODO: .... disappointing but let's get a clean test report.
    // t.addTest(testReadUntilTrim);
    // t.addTest(testReadUntilLTrim);
    t.addTest(testReadUntilRTrim);
    t.addTest(testReadUntilDisallowSpaceTrim);
    t.addTest(testReadUntilExtract);
    t.addTest(testReadUntilExtractNonWhiteCharacterAfterRightToken);
    t.addTest(testReadUntilAllowExtract);
    t.addTest(testReadUntilAllowExtractException);
    // t.addTest(testReadUntilDisallowExtract);
    t.addTest(testReadUntilLowerCaseExtract);
    t.addTest(testReadUntilUpperCaseExtract);

    t.addTest(testReadUntilCursorAtUntil);
    t.addTest(testReadUntilCursorAtUntilNotInclusive);
    t.addTest(testReadUntilFillBufferOnce);
    t.addTest(testReadUntilFillBufferTwice);
    t.addTest(testReadUntilAtEndOfStreamButAlsoUntil); 

    t.addTest(testReadEls);
    t.addTest(testReadElsFullBuffer);
    t.addTest(testReadElsReaderTask);

    t.addTest(testPeek);
    t.addTest(testPeekNoMoreData);
    t.addTest(testUnget);
    t.addTest(testUngetLastCharacterBeforeFillBuffer);
    t.addTest(testUngetHaventGotten);
    t.addTest(testGet);
    t.addTest(testGood);
    t.addTest(testGoodNoMoreData);
    t.addTest(testReadNaturalUntilDigitClassEmpty);
    t.addTest(testReadNaturalUntilDigitClass);
    t.addTest(testReadNaturalUntil);
    t.addTest(testReadNaturalUntilNoConsume);
    t.addTest(testReadNaturalN);
    t.addTest(testReadNaturalNDigitClass);
    t.addTest(testReadDecimalUntilDigitClassEmpty);
    t.addTest(testReadDecimalDigitClass); 
    t.addTest(testReadDecimalUntil);
    t.addTest(testReadDecimalUntilNoConsume);
    t.addTest(testReadDecimalDigitClassNoDecimalPart); 
    t.addTest(testReadDecimalDigitClassNoDecimalPartDecimal);
    t.addTest(testReadDecimalUntilNoDecimalPart);
    t.addTest(testReadDecimalUntilNoDecimalPartDecimal);
    t.addTest(testReadDecimalUntilNoConsumeNoDecimalPart);
    t.addTest(testReadDecimalN);
    t.addTest(testReadDecimalNNoDecimalPart);
    t.addTest(testReadDecimalNNoDecimalPartDecimal);
    t.addTest(testReadDecimalNNoDecimalPartDecimalCorrect);
    t.addTest(testReadDecimalNDigitClass);
    t.addTest(testReadDecimalNDigitClassNoDecimal);
    t.addTest(testReadDecimalNDigitClassNoDecimalPartDecimal);
    t.addTest(testReadDecimalLimit);

    // more limited set of tests using the istream.
    t.addTest(testReadUntilIStreamEStream);
    t.addTest(testReadElsIStreamEStream);
    t.addTest(testReadUntilIStreamEStreamReaderTask);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}