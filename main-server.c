#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "server.h"
#include "reader.h"

// TODO: how to keep imports clean? don't want to rely on preprocessor stuff.?

#define READ_BUFFER_SIZE 8096
#define FIELD_MAX 64

typedef struct http_field {
    char * name;
    char * value;
} http_field;

typedef struct http_request {
    char * method;
    char * path;
    char * version;
    // TODO: does static array contribute to stack usage?
    http_field fields[FIELD_MAX];
    int content_length;
    uint8_t * content;
} http_request;

int new_request(http_request * request, reader * reader) {
    printf("here 1?\n");

    if (request == NULL || reader == NULL) {
        goto ERROR;
    }

    request->method = reader_read_until(reader, ' '); // TODO: error check... if NULL, then read error
    if (request->method == NULL) {
        goto READ_ERROR;
    }
    request->path = reader_read_until(reader, ' ');
    if (request->path == NULL) {
        goto READ_ERROR;
    }
    request->version = reader_read_until(reader, '\n');
    if (request->version == NULL) {
        goto READ_ERROR;
    }

    request->content_length = -1;
    int field_idx = 0; 
    // for now, let's assume if no data in buffer by this point there won't be
    // no bound checking fornow...
    printf("here?\n");
    while (field_idx < FIELD_MAX && reader->bytes_in_buffer > 0 && !(reader->buf[reader->cursor] == '\n' || (reader->buf[reader->cursor] == '\r' && reader->buf[reader->cursor + 1] == '\n'))) {
        char * field_name = reader_read_until(reader, ':');
        if (field_name == NULL) {
            goto READ_ERROR;
        }
        // printf("adding field_name: %s---END---\n", field_name);
        char * value = reader_read_until(reader, '\n');
        if (value == NULL) {
            goto READ_ERROR;
        }
        request->fields[field_idx++] = (http_field){.name = field_name, .value = value};

        if (strcmp(field_name, "Content-Length") == 0) {
            request->content_length = atoi(value);
        }
    }

    printf("nalknsldknalk-0x%x,0x%x-------\n", reader->buf[reader->cursor], reader->buf[reader->cursor + 1]);
    // TODO: make sure we don't go over... implement in reader??
    if (reader->buf[reader->cursor] == '\n') {
        reader->cursor += 1;
    } else if (reader->buf[reader->cursor + 1] == '\n') {
        reader->cursor += 2;
    }

    if (field_idx == FIELD_MAX) {
        // TODO: error... not enough space for all those fields... too many fields stop it...
        printf("ERROR 1\n");
        goto ERROR;
    }

    if (request->content_length != -1) {
        request->content = reader_read_bytes(reader, request->content_length);
        if (request->content == NULL) {
            goto READ_ERROR;
        }
    }
    return 0;
READ_ERROR:
    return -1;
ERROR:
    return -2;
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
        // TODO: add field for cookies
        // TODO: add a separate field to request struct for this?
        printf("%s: %s\n", field_name, r.fields[idx].value);
        idx++;
        field_name = r.fields[idx].name;
    }

    if (r.content != NULL && r.content_length > 0) {
        printf("---CONTENT_START(%d)---\n", r.content_length);
        for (int i = 0; i < content_length; i++) {
            printf("%c", r.content[i]);
        }
    }
    printf("---END\n");
}

// # types
void * http_connection_handler(int * conn_fd) {
    // read and parse http header....
    // do stuff, write response...
    // voila ....

    // TODO:
    // dereference right away so that caller can do whatever with memory....

    // TODO: this is all being initialized to zero automatically... is this a compiler option????
    reader reader = {0}; // explicitly initialize to zero, I think {0} == {};
    reader_initialize(&reader, *conn_fd);

    http_request r;
    // TODO: use convention when handling errors where possible... should I return error, leverage errno, pass in error type as reference...
    // same for other outputs....
    int err = new_request(&r, &reader);
    if (err != 0) {
        printf("ERROR PROCESSING REQUEST\n");
        goto CLEANUP_REQUEST;
    }
    print_request(r);
CLEANUP_REQUEST:
    delete_request(&r);
CLEANUP:
    close(reader.fd); // doc's say you shouldn't retry close so ignore ret
    return NULL;
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