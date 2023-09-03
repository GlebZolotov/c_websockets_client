all: websockets_client.so

ws.o: websockets.c
	gcc -c -Wall $< -o $@ -Ilibwebsockets/build/include -Llibwebsockets/build/lib -lwebsockets
websockets_client.so: ws.o 
	gcc $^ -o $@ -Ilibwebsockets/build/include -Llibwebsockets/build/lib -lwebsockets -fPIC -shared
