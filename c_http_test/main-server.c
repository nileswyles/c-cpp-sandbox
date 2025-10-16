#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "server.h"
#include "iostream.h"

// TODO: how to keep imports clean? don't want to rely on preprocessor stuff.?

#define FIELD_MAX 64

typedef struct http_field {
    char * name;
    char * value;
} http_field;

typedef struct http_request {
    char * method;
    char * path;
    char * version;
    http_field fields[FIELD_MAX];
    int content_length;
    uint8_t * content;
} http_request;

int new_request(http_request * request, reader * reader) {
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
    int peek = reader_peek_for_empty_line(reader);
    while (field_idx < FIELD_MAX && peek == 0) {
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
            // TODO: negatives? use strtoul instead?
            request->content_length = atoi(value);
        }
        peek = reader_peek_for_empty_line(reader);
    }
    
    if (peek == -1) {
        goto READ_ERROR;
    }

    if (field_idx == FIELD_MAX) {
        // TODO: error... not enough space for all those fields... too many fields stop it...
        // printf("ERROR 1\n");
        goto ERROR;
    }

    if (request->content_length != -1) {
        // printf("request->content: %lx\n", request->content);
        request->content = reader_read_bytes(reader, request->content_length);
        // printf("request->content: %lx\n", request->content);
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
        for (int i = 0; i < r.content_length; i++) {
            printf("%c", (char)r.content[i]);
        }
    }
    printf("---END\n");
}

// # types
uint8_t http_connection_handler(int conn_fd) {
    reader * reader = reader_constructor(conn_fd, READER_RECOMMENDED_BUF_SIZE); // explicitly initialize to zero, I think {0} == {};

    http_request r;
    int err = new_request(&r, &reader);
    if (err != 0) {
        printf("ERROR PROCESSING REQUEST\n");
        goto CLEANUP;
    }
    print_request(r);
CLEANUP:
    reader_destructor(reader);
    delete_request(&r);
    close(conn_fd); // doc's say you shouldn't retry close so ignore ret
    return 1;
}

int main(int argc, char * argv[]) {
    int ret = -1;
    if (argc == 3) {
        // DEDEDEDE
        serverListen(argv[1], atoi(argv[2]), http_connection_handler);
        ret = 0;
    } 
    return ret;
}