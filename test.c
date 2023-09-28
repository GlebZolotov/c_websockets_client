#include "websockets.h"

int main() {
    connection conn = ws_connect("wss", "stream.binance.com", 9443, "/stream?streams=adausdt@depth20@100ms/btcusdt@depth20@100ms", true, 60);
    int max_len = 0;
    while(conn.web_socket != NULL) {
        message msg = ws_recv(conn);
        if (msg.len > max_len) {
            max_len = msg.len;
        }
        printf("%d\n%s\n", max_len, (char*)msg.data);
    }
    ws_close(conn);
}