#include "reader.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TIME_TO_WAIT_FOR_READ_MS 10000
#define TIME_TO_WAIT_AFTER_READ_DATA_MS 100

// TODO: ******** TOGGLABLE DEBUG LOGGER AND GOOD DEBUG PRINTSTATEMENTS ***********
// and stop using int, use int8_t, uint8_t, size_t etc instead

static int fill_buffer(reader * const r);
static bool cursor_check(reader * const r);

// this may seem like much for this obj, but let's get in the habit of doing this...
reader * reader_constructor(const int fd) {
    reader * r = (reader *)malloc(sizeof(reader));
    reader_initialize(r, fd);
    return r;
}

void reader_destructor(reader * const r) {
    free(r);
}

void reader_initialize(reader * const r, const int fd) {
    // TODO: this is all being initialized to zero automatically... is this a compiler option????
    // printf("r.cursor, %d\n", r->cursor);
    // for (int i = 0; i < READ_BUFFER_SIZE; i++) {
    //     printf("%x,", r->buf[i]);
    // }
    // printf("\n");
    r->cursor = 0;
    r->fd = fd;
    r->bytes_in_buffer = 0;
}

// -1 == read error
// 0 == not empty line
// 1 == is empty line
int reader_peek_for_empty_line(reader * const r) {
    if (!cursor_check(r)) {
        return -1;
    }
    if (r->buf[r->cursor] == '\n') {
        r->cursor += 1;
        return 1;
    } else if (r->cursor + 1 >= r->bytes_in_buffer) { 
        // read and append to buffer...
        uint8_t * tmp[READ_BUFFER_SIZE - 1];
        ssize_t ret = read(r->fd, tmp, READ_BUFFER_SIZE - 1);
        if (ret < 1 || ret > (READ_BUFFER_SIZE - 1)) {
            return -1;
        } else {
            r->buf[0] = r->buf[r->cursor];
            memcpy(r->buf + 1, tmp, ret);
        }
    } 
    // should be safe now...
    if (r->buf[r->cursor] == '\r' && r->buf[r->cursor + 1] == '\n') {
        r->cursor += 2;
        return 1;
    }
    return 0;
}

// if return == NULL, check errno for read error.
// TODO: should I change ret type to char * and allocate the extra byte, in the event this data is actually a string?
// Update to be g++ compatible?
uint8_t * reader_read_bytes(reader * const r, const uint32_t n) {
    if (!cursor_check(r)) {
        // if read error..
        return NULL; 
    }

    uint32_t bytes_read = 0;
    // TODO: limit size of this buffer?
    uint8_t * const data = (uint8_t *)malloc(n*sizeof(uint8_t));
    uint8_t * data_cursor = data;
    while (bytes_read < n) {
        int bytes_left_to_read = n - bytes_read;
        int bytes_left_in_buffer = r->bytes_in_buffer - r->cursor;
        // printf("bytes_read %d, bytes_to_read %d, bytes_left_to_read %d, bytes_left_in_buffer %d\n", bytes_read, n, bytes_left_to_read, bytes_left_in_buffer);

        if (bytes_left_to_read > bytes_left_in_buffer) {
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
            // printf("cursor: %lx, start: %lx\n", data_cursor, data);
            memcpy(data_cursor, r->buf + r->cursor, bytes_left_to_read);
            r->cursor += bytes_left_to_read;
            data_cursor += bytes_left_to_read;
            bytes_read += bytes_left_to_read; // no more loop after this...
        }
    } 

    // printf("START---");
    // for (int i = 0; i < n; i++) {
    //     printf("%c", data[i]);
    // }
    // printf("---END\n");
    return data;
}

// if return == NULL, check errno for read error.
char * reader_read_until(reader * const r, const char until) {
    if (!cursor_check(r)) {
        return NULL; 
    }

    int start_cursor = r->cursor;
    printf("start_cursor, %d, %c\n", start_cursor, r->buf[start_cursor]);
    char c = (char)r->buf[start_cursor];
    char * s = NULL;
    size_t s_size = 0;
    while(c != until) {
        if (r->cursor == r->bytes_in_buffer) { // if cursor pointing past data...
            // copy all data in buffer to string and read more
            int bytes_left_in_buffer = r->bytes_in_buffer - start_cursor;
            if (s != NULL) {
                printf("reached end of buffer, old string: ");
                for (int i = 0; i < s_size; i++) {
                    printf("%c", s[i]);
                }
                printf("\n");
            }
            char * const new_s = (char *)malloc((s_size+bytes_left_in_buffer)*sizeof(char));
            if (new_s == NULL) {
                // TODO: again, better error handling
                printf("NEW_S == NULL\n");
                free(s);
                return NULL;
            }
            memcpy(new_s, s, s_size);
            memcpy(new_s + s_size, r->buf + start_cursor, bytes_left_in_buffer);
            free(s);

            s = new_s;
            s_size += bytes_left_in_buffer;
            printf("reached end of buffer, new string: ");
            for (int i = 0; i < s_size; i++) {
                printf("%c", s[i]);
            }
            printf("\n");

            int ret = fill_buffer(r);
            if (ret == -1) {
                free(s);
                return NULL;
            }
            start_cursor = r->cursor; // == 0
        }
        c = (char)r->buf[r->cursor++]; // we point at character past 'until' at the end of this loop
        // printf("%c", c);
    }

    // need to append up until to string...
    int bytes_up_until_cursor = r->cursor - start_cursor;
    if (bytes_up_until_cursor == 0) {
        // if cursor is already pointing at 'until', increment cursor because it didn't enter loop.. this will yield in an empty string
        r->cursor++;
    }
    char * const new_s = (char *)malloc((s_size + bytes_up_until_cursor)*sizeof(char));
    if (new_s == NULL) {
        // TODO: again, better error handling
        printf("NEW_S 2 == NULL\n");
        free(s);
        return NULL;
    }
    memcpy(new_s, s, s_size);
    memcpy(new_s + s_size, r->buf + start_cursor, bytes_up_until_cursor); // copy form start of buffer to cursor
                                    // if cursor == 0; then it should copy nothing?
                                    // if cursor == 1; then it should copy byte at index 0
                                    // if cursor == 2; then it should copy bytes at index 0 and 1.
    free(s);
    // s_size or bytes_up_until_cursor should be at least 1 at this point...
    new_s[s_size+bytes_up_until_cursor-1] = 0; // replace until with NUL byte... -1 because size starts at 1 not index 0.
    printf("end_cursor, %d, %c\n", r->cursor, r->buf[r->cursor]);
    printf("new string: %s---END\n", new_s);
    return new_s;
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
        ssize_t res = read(fd, buf, READ_BUFFER_SIZE);
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

static int fill_buffer(reader * const r) {
    r->cursor = 0;
    ssize_t ret = read(r->fd, r->buf, READ_BUFFER_SIZE);
    // TODO: retry on EAGAIN?, revisit possible errors...
    if (ret == -1 || ret > READ_BUFFER_SIZE) {
        r->bytes_in_buffer = 0; // uint
    } else {
        r->bytes_in_buffer = ret;
    }
    return ret;
}

static bool cursor_check(reader * const r) {
    if (r->cursor >= r->bytes_in_buffer) { // if read past buffer
        if (fill_buffer(r) < 1) {
            return false;
        }
    }
    return true;
}