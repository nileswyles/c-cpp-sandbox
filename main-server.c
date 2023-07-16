#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "server.h"

#define READ_BUFFER_SIZE 8096
#define FIELD_MAX 64

// TODO: implement lowercase and trimming support....
char * read_until(uint8_t ** p, char until, int * buf_size) {
    int count = 0;
    uint8_t * buf = *p;
    char prev;
    while (count < *buf_size) {
        uint8_t c = buf[count];
        // printf("%c", (char)c);
        // until char or reached end of buffer...
        if (c == until) {
            break;
        }
        count++;
        prev = c;
    } // note, if this doesn't find 'until', it returns a string of whatever data it found... this should probably be an error instead...
    // return how many bytes to copy from buf start...

    // printf("found %c, at %d\n", until, count);
    // TODO: no limit to how much memory is allocated? is this an issue?
    // TODO: malloc err checking? 

    int bytes = count;
    if (prev == '\r' && until == '\n') {
        bytes--; // ignore \r
    }
    // int bytes = count;
    char * s = malloc(bytes * sizeof(char));
    memcpy(s, buf, bytes);
    s[bytes] = 0; // NULL TERMINATE.. instead of whatever character we read until

    printf("\n\n-------copied string, %s------\n\n-", s);

    // increment pointer... we want to move past the until so increment by count + 1 (not bytes)
    *p += count + 1;
    *buf_size -= (count + 1); // subtract bytes read from running buf size......
    // TODO: bounds checking... though not currently an issue because content
    return s;
}

typedef struct http_field {
    char * name;
    char * value;
} http_field;

typedef struct http_request {
    char * method;
    char * path;
    char * version;
    http_field fields[FIELD_MAX];
    uint8_t * content;
} http_request;

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
int read_request(int fd, uint8_t ** p) {
    uint8_t buf[READ_BUFFER_SIZE];
    int bytes_read = 0;
    int res = read(fd, buf, READ_BUFFER_SIZE);
    // printf("read_request, %d, %d, %d\n", res, bytes_read, errno);

    // yuckk... 
    int loop_count = 0;
    bool read_something = false;
    while (res > 0 || (loop_count < 10 && errno == 11)) {
        // TODO: should I parse while reading????.... this is gonna need some work..
        // allocate memory for old + new + ETX
        if (res > 0) {
        // printf("read something\n");
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

        } else if (read_something) {
            break;
        } else {
            sleep(1);
        }
        // printf("stuck here huh? shouldn't this return zero and exit the loop?\n");
        res = read(fd, buf, READ_BUFFER_SIZE);
        // printf("read_request, %d, %d, %d\n", res, bytes_read, errno);
        loop_count++;
    }
    return bytes_read;
}

void print_buffer(uint8_t * p, int size) {
    for (int i = 0; i < size; i++) {
        printf("%c",(char)p[i]);
    }
    printf("\n");
}

int new_request(http_request * r, uint8_t * buf, int buf_size) {
    // p -> address of first byte.
    // buf_ptr -> address of p on initialization and address of new p after each call to read_until.
    print_buffer(buf, buf_size);
    // TODO: buf_size or pointer to end, to prevent overflow? or check for ETX?
    r->method = read_until(&buf, ' ', &buf_size);
    r->path = read_until(&buf, ' ', &buf_size);
    r->version = read_until(&buf, '\n', &buf_size);

    int content_length = -1;
    int field_idx = 0; 
    // read fields until FIELD_MAX or empty line (\n, or \r\n)
    printf("0x%x,0x%x----\n", buf[0], buf[1]);
    while (field_idx < FIELD_MAX && buf_size > 0 && !(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))) {
        char * field_name = read_until(&buf, ':', &buf_size);
        // printf("adding field_name: %s---END---\n", field_name);
        char * value = read_until(&buf, '\n', &buf_size);

        r->fields[field_idx++] = (http_field){.name = field_name, .value = value};

        if (strcmp(field_name, "Content-Length") == 0) {
            content_length = atoi(value);
        }
        // printf("0x%x,0x%x-------\n", buf[0], buf[1]);
    }

    printf("nalknsldknalk-0x%x,0x%x-------\n", buf[0], buf[1]);

    // TODO: consolidate all of the below
    // consume empty line
    if (field_idx < FIELD_MAX && buf_size > 0) {
        if (buf[0] == '\r') {
            buf += 1;
            buf_size -= 1;
        }
        buf += 1;
        buf_size -= 1;
        // TODO: overflow check, might insert ETX when we read (see above)...
    }

    printf("after-0x%x,0x%x, %d-------\n", buf[0], buf[1], buf_size);
    // TODO: better error handling??? revisit this...
    if (field_idx == FIELD_MAX && (buf_size == 0 || !(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n')))) {
        // TODO: error... not enough space for all those fields...
        printf("ERROR 1\n");
        return -1;
    }

    if (content_length == -1 && buf_size > 0) {
        // TODO: error... something went wrong... no content length field but there's data.... 
        printf("ERROR 2\n");
        return -1;
    } else if (content_length != -1) {
        if (buf_size != content_length) {
        // TODO: error... something went wrong... more data in the buffer than specified by content_length header.... 
            printf("IN HERE DAWG\n");
            printf("ERROR 3, %d,%d\n", buf_size, content_length);
            return -1;
        } else {
            printf("IN HERE DAWG2\n");
            // balbksdlasldblsabdbl not true?
            // no need to copy just give original read pointer... we can clear when we dump the request.
            printf("after-0x%x,0x%x, %d-------\n", buf[0], buf[1], buf_size);
            
            r->content = malloc(content_length * sizeof(uint8_t));
            memcpy(r->content, buf, content_length);
        } 
    }
    return 0;
}

void delete_request(http_request * r) {
    free(r->method);
    free(r->path);
    free(r->version);
    for (int i = 0; i < FIELD_MAX; i++) {
        free(r->fields[i].name);
        free(r->fields[i].value);
    }
    free(r->content);
}

void print_request(http_request r) {
    printf("%s %s %s\n", r.method, r.path, r.version);

    int content_length = 0;
    int idx = 0;
    char * field_name = r.fields[idx].name;
    while(field_name != NULL && idx < FIELD_MAX) {
        // TODO: add a separate field to request struct for this?
        // TODO: add field for cookies
        if (strcmp(field_name, "Content-Length") == 0) {
            content_length = atoi(r.fields[idx].value);
        }
        printf("%s: %s\n", field_name, r.fields[idx].value);
        idx++;
        field_name = r.fields[idx].name;
    }
    printf("\n");

    if (r.content != NULL && content_length > 0) {
        for (int i = 0; i < content_length; i++) {
            printf("%c", r.content[i]);
        }
        printf("\n");
    }
}


void http_connection_handler(int * conn_fd) {
    // read and parse http header....
    // do stuff, write response...
    // voila ....

    uint8_t * buf;
    // printf("%d,%d,%d\n", &buf_ptr, buf_ptr, *buf_ptr);
    int bytes_read = read_request(*conn_fd, &buf);
    // printf("%x,%x,%x,%x\n", &buf_ptr, buf_ptr, *buf_ptr, **buf_ptr);
    printf("%x\n", buf);
    if (bytes_read == 0) {
        // check errno, and handle err... 
        // should we free here or in readRequest??? probably yes now...
        free(buf);
        return;
    } // else done, reading request.. should have full request...
    // TODO: I am effectively reading until EOF... revisit

    http_request r;
    int err = new_request(&r, buf, bytes_read);
    if (err != 0) {
        // process request...
        printf("ERROR PROCESSING REQUEST\n");
    }
    print_request(r);

    // cleanup
    delete_request(&r);
    free(buf);
    int ret = close(*conn_fd);
    // pthread_exit();
}

int main(int argc, char * argv[]) {
    int ret = -1;
    if (argc == 3) {
        // DEDEDEDE
        server_listen(argv[1], atoi(argv[2]), http_connection_handler);
        ret = 0;
    } 
    return ret;
}