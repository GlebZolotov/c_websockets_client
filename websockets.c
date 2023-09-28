#include "websockets.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE (1000000)

typedef struct payload
{
	unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + BUFFER_SIZE + LWS_SEND_BUFFER_POST_PADDING];
    size_t len;
    bool is_write;
    struct payload *next;
} payload;

static bool is_connected;
static int msg_count;
static bool recv_from_buf;
static int error_code;

static int callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
            is_connected = true;
            //printf("Connection is established\n");
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
            payload * pd = (payload *)(lws_context_user(lws_get_context(wsi)));
            if (pd->is_write) break;

            int count_of_msg = msg_count;
            while(count_of_msg > 0) {
                if (pd->next == NULL) {
                    pd->next = (payload *)malloc(sizeof(payload));
                    pd->next->next = NULL;
                    pd->next->len = 0;
                }
                pd = pd->next;
                count_of_msg--;
            }

            //printf("I get a message from the server with len %ld\n", len);
            lws_set_timer_usecs(wsi, -1);
            if (pd->len + len < BUFFER_SIZE) {
                memcpy( &pd->data[LWS_SEND_BUFFER_PRE_PADDING + pd->len], in, len );
                pd->len += len;
            } else {
                memcpy(&pd->data[LWS_SEND_BUFFER_PRE_PADDING], in, len);
                pd->len = len;
                error_code = -1;
                is_connected = false;
			    return -1;
            }

            if (lws_is_final_fragment(wsi)) {
                msg_count++;
            }

			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
            payload * pd = (payload *)(lws_context_user(lws_get_context(wsi)));
            if (!(pd->is_write)) break;
            //printf("I send a message to the server\n");
            lws_write( wsi, &(pd->data[LWS_SEND_BUFFER_PRE_PADDING]), pd->len, LWS_WRITE_TEXT );
            pd->len = 0;
			break;
		}

        case LWS_CALLBACK_TIMER:
            lws_cancel_service(lws_get_context(wsi));
            break;
		case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CLOSED:
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            //printf("web_socket closed\n");
            is_connected = false;
			break;

		default:
            //printf("Code %d\n", reason);
			break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
                    {
                        "callback",
                        callback,
                        sizeof(payload),
                        0,
                    },
                    { NULL, NULL, 0, 0 } /* terminator */
                };

static struct lws_extension extensions[] = {
                    {
                            "permessage-deflate", lws_extension_callback_pm_deflate,
                            "permessage-deflate" "; client_no_context_takeover"
                            "; client_max_window_bits"
                    },
                    { NULL, NULL, NULL /* terminator */ }
                };

connection ws_connect(const char* protocol, const char* host, unsigned int port, const char* path, bool is_permessage_deflate, unsigned int read_timeout) {
    is_connected = false;
    error_code = 0;
    msg_count = 0;
    recv_from_buf = false;
    payload *receive_payload = (payload *)malloc(sizeof(payload));
    receive_payload->next = NULL;
    connection conn;
    conn.web_socket = NULL;
    conn.context = NULL;

    struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
    if (is_permessage_deflate) {
        info.extensions = extensions;
    }
    if (strcmp(protocol, "wss") == 0) {
        info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    }
    info.user = receive_payload;

	conn.context = lws_create_context( &info );

    struct lws_client_connect_info ccinfo = {0};
    ccinfo.context = conn.context;
    ccinfo.address = host;
    ccinfo.port = port;
    ccinfo.path = path;
    ccinfo.host = host;
    ccinfo.origin = host;
    ccinfo.protocol = protocols[0].name;
    if (strcmp(protocol, "wss") == 0) {
        ccinfo.ssl_connection = LCCSCF_USE_SSL;
    }
    conn.web_socket = lws_client_connect_via_info(&ccinfo);

    while(!is_connected) {
        lws_service( conn.context, /* timeout_ms = */ 0 );
    }
    conn.read_timeout = read_timeout;

    return conn;
}

void ws_close(connection conn) {
    payload * pd = lws_context_user(conn.context);
    while(pd->next != NULL) {
        payload * for_del = pd->next;
        pd->next = for_del->next;
        free(for_del);
    }
    free(pd);
    lws_context_destroy( conn.context );
}

void ws_send(connection conn, message msg) {
    payload * pd = (payload *)(lws_context_user(conn.context));
    pd->is_write = true;
    pd->len = msg.len;
    memcpy(&(pd->data[LWS_SEND_BUFFER_PRE_PADDING]), msg.data, msg.len);
    lws_callback_on_writable( conn.web_socket );
    while (pd->len > 0) {
        lws_service( conn.context, /* timeout_ms = */ 0 );
    }
}

message ws_recv(connection conn) {
    payload * pd = (payload *)(lws_context_user(conn.context));
    message res;
    if (recv_from_buf) {
        payload * for_del = pd->next;
        pd->next = for_del->next;
        free(for_del);
    }
    if (msg_count > 0) {
        pd = pd->next;
        res.len = pd->len;
        pd->data[LWS_SEND_BUFFER_PRE_PADDING + pd->len] = 0;
        res.data = &pd->data[LWS_SEND_BUFFER_PRE_PADDING];
        res.error_code = error_code;
        msg_count--;
        recv_from_buf = true;
        return res;
    }
    pd->is_write = false;
    pd->len = 0;
    lws_set_timer_usecs(conn.web_socket, conn.read_timeout * 1000000);
    while (pd->len == 0) {
        lws_service( conn.context, /* timeout_ms = */ 0 );
    }
    res.len = pd->len;
    pd->data[LWS_SEND_BUFFER_PRE_PADDING + pd->len] = 0;
    res.data = &pd->data[LWS_SEND_BUFFER_PRE_PADDING];
    res.error_code = error_code;
    msg_count--;
    recv_from_buf = false;
    return res;
}


