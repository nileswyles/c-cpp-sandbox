#include "reader.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TIME_TO_WAIT_FOR_READ_MS 10000
#define TIME_TO_WAIT_AFTER_READ_DATA_MS 100

// called when cursor == READ_BUFFER_SIZE.. cursor is then reset to 0
static int fill_buffer(reader * r) {
    r->cursor = 0;
    int ret = read(r->fd, r->buf, READ_BUFFER_SIZE);
    if (ret == -1 || ret > READ_BUFFER_SIZE) {
        r->bytes_in_buffer = 0;
        return -1;
    } else {
        r->bytes_in_buffer = ret;
    }
}

void reader_initialize(reader * r, int fd) {
    // TODO: this is all being initialized to zero automatically... is this a compiler option????
    // printf("r.cursor, %d\n", r->cursor);
    // for (int i = 0; i < READ_BUFFER_SIZE; i++) {
    //     printf("%x,", r->buf[i]);
    // }
    // printf("\n");
    r->cursor = -1;
    r->fd = fd;
    r->bytes_in_buffer = -1;
}

// if return == NULL, check errno for read error.
uint8_t * reader_read_bytes(reader * r, int n) {
    if (r->cursor == r->bytes_in_buffer) { // if read past buffer
        int ret = fill_buffer(r);
        if (ret == -1) {
            return NULL;
        }
    }

    int bytes_read = 0;
    int sleep_before_read = 0;
    int sleep_after_read = 0;
    bool read_something = false;
    // TODO: limit size of this buffer?
    uint8_t * data = malloc(n*sizeof(uint8_t));
    uint8_t * data_cursor = data;
    while (bytes_read < n) {
        int bytes_left_to_read = n - bytes_read;
        int bytes_left_in_buffer = r->bytes_in_buffer - r->cursor;

        if (bytes_left_to_read >= bytes_left_in_buffer) {
            // copy data left in buffer and read more
            memcpy(data_cursor, r->buf + r->cursor, bytes_left_in_buffer);
            bytes_read += bytes_left_in_buffer;
            data_cursor += bytes_left_in_buffer;

            int ret = fill_buffer(r);
            if (ret == -1) {
                // TODO:
                // If an input or output function blocks for this period of time, and data has been sent or received, the return value of that function will be the amount of data transferred; if no data has been transferred and the timeout has been reached then -1 is returned with errno set to EAGAIN or EWOULDBLOCK, or EINPROGRESS
                free(data);
                return NULL;
            }
        } else {
            // else enough data in buffer
            memcpy(data_cursor, r->buf + r->cursor, bytes_left_to_read);
            r->cursor += bytes_left_to_read;
            data_cursor += bytes_left_to_read;
            bytes_read += bytes_left_to_read; // no more loop after this...
        }
    } 

    return data;
}

// if return == NULL, check errno for read error.
char * reader_read_until(reader * r, char until) {
    if (r->cursor == r->bytes_in_buffer) { // read past buffer
        int ret = fill_buffer(r);
        if (ret == -1) {
            return NULL;
        }
    }

    int string_size = 1; // size including NUL byte
    int start_cursor = r->cursor;
    char c;
    char * s = "";
    while(c != until) {
        c = (char)r->buf[r->cursor++];
        string_size++;
        if (r->cursor == r->bytes_in_buffer) { // if cursor pointing past data...
            // copy all data in buffer to string and read more
            int bytes_left_in_buffer = r->bytes_in_buffer - start_cursor;
            char * new_s = malloc((string_size+bytes_left_in_buffer)*sizeof(char));
            memcpy(new_s, s, string_size-1); // don't copy NUL byte
            memcpy(new_s, r->buf + start_cursor, bytes_left_in_buffer);
            free(s);
            s = new_s;
            string_size += bytes_left_in_buffer;
            s[string_size] = 0; // NUL terminate...

            int ret = fill_buffer(r);
            if (ret == -1) {
                free(s);
                return NULL;
            }
            start_cursor = r->cursor; // == 0
        }
    }

    // need to append up until to string...
    int bytes_up_until_cursor = r->cursor;
    char * new_s = malloc((string_size + bytes_up_until_cursor)*sizeof(char));
    memcpy(new_s, s, string_size-1); // Don't copy NUL byte
    memcpy(new_s, r->buf, bytes_up_until_cursor); // copy form start of buffer to cursor
                                    // if cursor == 0; then it should copy nothing?
                                    // if cursor == 1; then it should copy byte at index 0
                                    // if cursour == 2; then it should copy bytes at index 0 and 1.
    free(s);
    s = new_s;
    s[string_size] = 0; // NUL terminate...

    return s;
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
int read_chunk_non_blocking_fd(int fd, uint8_t ** p) {
    uint8_t * buf[READ_BUFFER_SIZE];
    int bytes_read = 0;
    int sleep_before_read = 0;
    int sleep_after_read = 0;
    bool read_something = false;
    // TODO: limit size of this buffer?
    while (1) {
        int res = read(fd, buf, READ_BUFFER_SIZE);
        if (res > 0) {
            read_something = true;
            uint8_t * new_p = malloc((bytes_read + res)*sizeof(uint8_t));
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