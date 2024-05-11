#include "reader.h"
#include "logger.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace WylesLibs;

#define TIME_TO_WAIT_FOR_READ_MS 10000
#define TIME_TO_WAIT_AFTER_READ_DATA_MS 100

// TODO: ******** TOGGLABLE DEBUG LOGGER AND GOOD DEBUG PRINTSTATEMENTS ***********
// and stop using int, use int8_t, uint8_t, size_t etc instead

// -1 == read error
// 0 == not empty line
// 1 == is empty line
int Reader::peekForEmptyLine() {
    if (!cursorCheck()) {
        return -1;
    }
    if (this->buf[this->cursor] == '\n') {
        this->cursor += 1;
        return 1;
    } else if (this->cursor + 1 >= this->bytes_in_buffer) { 
        // read and append to buffer...
        uint8_t * tmp[this->buf_size - 1];
        ssize_t ret = read(this->fd, tmp, this->buf_size - 1);
        if (ret < 1 || (size_t)ret > (this->buf_size - 1)) {
            return -1;
        } else {
            this->buf[0] = this->buf[this->cursor];
            memcpy(this->buf + 1, tmp, ret);
        }
    } 
    // should be safe now...
    if (this->buf[this->cursor] == '\r' && this->buf[this->cursor + 1] == '\n') {
        this->cursor += 2;
        return 1;
    }
    return 0;
}

// if return == NULL, check errno for read error.
// TODO:
// Can probably be simplified to do 1 read and 1 copy but let's keep changes 'incremental'. 
//  Need to update tests before making that change. Which I don't feel like doing right now.
//  or by reading a single byte per read call, but that seems like the wrong way to do it.
Array<uint8_t> * Reader::readBytes(const size_t n) {
    if (!cursorCheck()) {
        // if read error..
        return NULL; 
    }

    Array<uint8_t> * data = new Array<uint8_t>(n+1);
    size_t bytes_read = 0;
    // TODO: limit size of this->buffer?
    while (bytes_read < n) {
        size_t bytes_left_to_read = n - bytes_read;
        size_t bytes_left_in_buffer = this->bytes_in_buffer - this->cursor;
        // printf("bytes_read %d, bytes_to_read %d, bytes_left_to_read %d, bytes_left_in_buffer %d\n", bytes_read, n, bytes_left_to_read, bytes_left_in_buffer);

        if (bytes_left_to_read > bytes_left_in_buffer) {
            // copy data left in buffer and read more
            data->append(this->buf + this->cursor, bytes_left_in_buffer);
            bytes_read += bytes_left_in_buffer;

            int ret = fillBuffer();
            if (ret == -1) {
                // TODO:
                // If an input or output function blocks for this->period of time, and data has been sent or received, the return value of that function will be the amount of data transferred; if no data has been transferred and the timeout has been reached then -1 is returned with errno set to EAGAIN or EWOULDBLOCK, or EINPROGRESS
                delete data;
                return NULL;
            }
        } else {
            // else enough data in buffer
            // printf("cursor: %lx, start: %lx\n", data_cursor, data);
            data->append(this->buf + this->cursor, bytes_left_to_read);
            this->cursor += bytes_left_to_read;
            bytes_read += bytes_left_to_read; // no more loop after this->..
        }
    } 
    // printf("START---");
    // for (int i = 0; i < n; i++) {
    //     printf("%c", copy->buf[i]);
    // }
    // printf("---END\n");
    return data;
}

// if return == NULL, check errno for read error.
Array<uint8_t> * Reader::readUntil(const char until) {
    if (!cursorCheck()) {
        return NULL; 
    }

    size_t start_cursor = this->cursor;
    loggerPrintf(LOGGER_DEBUG, "reader_read_until start_cursor: %lu\n", start_cursor);
    uint8_t c = this->buf[start_cursor];

    Array<uint8_t> * data = new Array<uint8_t>();
    while(c != until) {
        if (this->cursor == this->bytes_in_buffer) { // if cursor pointing past data...
            // copy all data in buffer to string and read more
            size_t bytes_left_in_buffer = this->bytes_in_buffer - start_cursor;
            data->append(this->buf + start_cursor, bytes_left_in_buffer);

            loggerPrintf(LOGGER_DEBUG, "reached end of buffer, flush buffer to string and read more:\n");
            loggerPrintByteArray(LOGGER_DEBUG, data->buf, data->size);

            int ret = fillBuffer();
            if (ret == -1) {
                delete data;
                return NULL;
            }
            start_cursor = this->cursor; // == 0
        }
        c = this->buf[++this->cursor]; 
    }
    this->cursor++;
    size_t bytes_up_until_cursor = this->cursor - start_cursor;
    data->append(this->buf + start_cursor, bytes_up_until_cursor);

    loggerPrintf(LOGGER_DEBUG, "reader_read_until end cursor: %lu\n", this->cursor);
    loggerPrintf(LOGGER_DEBUG, "reader_read_until string: %s\n", (char *)data->buf);

    return data;
}

// In the spirit of overcomplicating things....

// p == pointer to pointer of byte array
// *p = pointer to byte array
// uint8_t * p == pointer to byte array (first element)
// uint8_t ** y == pointer to afformentioned pointer.
// uint8_t * new_p = pointer to new byte array
// *y = new_p

// uint8_t val == val
// uint8_t *val == pointer to val
// uint8_t new_val = new_val
// *val = new_val
int Reader::read_chunk_non_blocking_fd(int fd, uint8_t ** p) {
    uint8_t * buf[4096];
    int bytes_read = 0;
    int sleep_before_read = 0;
    int sleep_after_read = 0;
    bool read_something = false;
    // TODO: limit size of this->buffer?
    while (1) {
        ssize_t res = read(fd, buf, 4096);
        if (res > 0) {
            read_something = true;
            uint8_t * new_p = (uint8_t *)malloc((bytes_read + res)*sizeof(uint8_t));
            // copy data from previous reads...
            memcpy(new_p, *p, bytes_read);
            // copy data from latest read....
            memcpy(new_p, buf, res);
            // increment byte read count
            bytes_read += res;
            // free old memory...
            free(*p);
            // update new pointer
            *p = new_p;
            // try reading more
        }
        if (!read_something) {
            if (sleep_before_read < TIME_TO_WAIT_FOR_READ_MS) {
                usleep(1000);
                sleep_before_read += 1;
            } else {
                // didn't read anything in time...
                break;
            }
        } else { // else if read_something (res > 0?)
            // continue trying to read until TIME_TO_WAIT_AFTER_READ_DATA_MS
            if (sleep_after_read < TIME_TO_WAIT_AFTER_READ_DATA_MS) {
                usleep(1000);
                sleep_after_read += 1;
            } else {
                // read data.
                break;
            }
        }
    }
    return bytes_read;
}

int Reader::fillBuffer() {
    this->cursor = 0;
    ssize_t ret = read(this->fd, this->buf, this->buf_size);
    // TODO: retry on EAGAIN?, revisit possible errors...
    if (ret == -1 || (size_t)ret > this->buf_size) {
        this->bytes_in_buffer = 0; // uint
    } else {
        this->bytes_in_buffer = ret;
    }
    return ret;
}

bool Reader::cursorCheck() {
    if (this->cursor >= this->bytes_in_buffer) { // if read past buffer
        if (fillBuffer() < 1) {
            return false;
        }
    }
    return true;
}