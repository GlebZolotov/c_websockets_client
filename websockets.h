#include "libwebsockets.h"
#include <stdbool.h>

#define BUF_SIZE 1000000

typedef struct connection {
    struct lws_context *context;
    struct lws *web_socket;
    unsigned int read_timeout;
} connection;

typedef struct message {
    size_t len;
    void* data;
    int error_code;
} message;

connection ws_connect(const char* protocol, const char* host, unsigned int port, const char* path, bool is_permessage_deflate, unsigned int read_timeout);

void ws_send(connection conn, message msg);

message ws_recv(connection conn);

void ws_close(connection conn);

