#include "reader.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define READ_BUFFER_SIZE 8096

// ehhh, should really read until a specific char but this will do for now...
#define TIME_TO_WAIT_FOR_READ_MS 10000
#define TIME_TO_WAIT_AFTER_READ_DATA_MS 100

static uint8_t buf[READ_BUFFER_SIZE];
static int cursor = 0;

// called when cursor == READ_BUFFER_SIZE.. cursor is then reset to 0
static int fill_buffer(int fd) {
    cursor = 0;
    return read(fd, buf, READ_BUFFER_SIZE);
}

// typedef reader struct {
//     int fd;
//     uint8_t buf[READ_BUFFER_SIZE];
//     int cursor;
// } reader;

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
int read_bytes(int fd, uint8_t ** p, int n) {
    int bytes_read = 0;
    int sleep_before_read = 0;
    int sleep_after_read = 0;
    bool read_something = false;
    // TODO: limit size of this buffer?
    *p = malloc(n*sizeof(uint8_t));

    uint8_t * p_cursor = *p;
    while (bytes_read < n) {
        // read from cursor to n or end of buffer...
        int bytes_left_to_read = n - bytes_read;
        if (cursor + bytes_left_to_read > READ_BUFFER_SIZE) {
            // if need to read more data...
            int bytes_left_in_buffer = READ_BUFFER_SIZE - cursor;
            memcpy(p_cursor, buf + cursor, bytes_left_in_buffer);
            bytes_read += bytes_left_in_buffer;
            p_cursor += bytes_left_in_buffer;

            // TODO: handle ret?
            // this resets cursor to zero
            int ret = fill_buffer(fd);
        } else {
            // else enough data in buffer
            memcpy(p_cursor, buf + cursor, bytes_left_to_read);
            cursor += bytes_left_to_read;
            p_cursor += bytes_left_to_read;
            bytes_read += bytes_left_to_read; // no more loop after this...
        }
    } 

    return bytes_read;
}

int read_chunk(int fd, uint8_t ** p) {
    uint8_t buf[READ_BUFFER_SIZE];
    int bytes_read = 0;
    int sleep_before_read = 0;
    int sleep_after_read = 0;
    bool read_something = false;
    // TODO: limit size of this buffer?
    while (1) {
        // TODO: allocate memory for old + new + ETX?
        // assuming FD is non-buffering
        int res = read(fd, buf, READ_BUFFER_SIZE);
        if (res > 0) {
            // printf("read something\n");
            read_something = true;
            uint8_t * new_p = malloc((bytes_read + res)*sizeof(uint8_t));
            // TODO: malloc err checking?
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
        // printf("stuck here huh? shouldn't this return zero and exit the loop?\n");
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

char * read_until(int fd, char until) {
    // TODO: timeout???
    int count = cursor;
    char c = (char)buf[count];
    char prev;
    // TODO: implement string type? because this is ehhh..
    int size = READ_BUFFER_SIZE;
    char * s = malloc(size*sizeof(char));
    while(c != until) {
        count++;
        prev = c;
        if (cursor == READ_BUFFER_SIZE) {
            // need to append what's left in buffer to string...
            // TODO: consolidate into function?
            int bytes_left_in_buffer = READ_BUFFER_SIZE - cursor;
            char * new_s = malloc(count*sizeof(char));
            memcpy(new_s, s, size);
            memcpy(new_s, buf, bytes_left_in_buffer);
            free(s);
            s = new_s;
            size = count;
            // TODO: handle ret?
            // this resets cursor to zero
            int ret = fill_buffer(fd);
        }
        c = (char)buf[count%READ_BUFFER_SIZE];
    }

    // move cursor to 1 past count, overflow should have already been handled above?
    cursor = count%READ_BUFFER_SIZE + 1;

    // need to append up until to string...
    char * new_s = malloc(count*sizeof(char));
    memcpy(new_s, s, size);
    memcpy(new_s, buf, cursor); // copy form start of buffer to cursor
                                    // if cursor == 0; then it should copy nothing?
                                    // if cursor == 1; then it should copy byte at index 0
                                    // if cursour == 2; then it should copy bytes at index 0 and 1.
    free(s);
    s = new_s;
    size = count; // update size for consitency... 

    return s;
}