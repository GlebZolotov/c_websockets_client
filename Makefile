all: websockets_client.so

ws.o: websockets.c libwebsockets/build/lib/libwebsockets.so
	gcc -c -Wall $< -o $@ -Ilibwebsockets/build/include -fPIC
websockets_client.so: ws.o libwebsockets/build/lib/libwebsockets.so
	gcc $^ -o $@ -Ilibwebsockets/build/include libwebsockets/build/lib/libwebsockets.so -fPIC -shared
test.o: test.c websockets_client.so
	gcc -c test.c -o test.o
test: test.o websockets_client.so
	gcc test.o websockets_client.so -o test

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: websockets_client.so libwebsockets/build/lib/libwebsockets.so
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 websockets_client.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libwebsockets/build/lib/libwebsockets.so $(DESTDIR)$(PREFIX)/lib/
