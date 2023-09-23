all: websockets_client.so

ws.o: websockets.c libwebsockets/build/lib/libwebsockets.so
	gcc -c -Wall $< -o $@ -Ilibwebsockets/build/include
websockets_client.so: ws.o libwebsockets/build/lib/libwebsockets.so
	gcc $^ -o $@ -Ilibwebsockets/build/include libwebsockets/build/lib/libwebsockets.so -fPIC -shared

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: websockets_client.so libwebsockets/build/lib/libwebsockets.so
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 websockets_client.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libwebsockets/build/lib/libwebsockets.so $(DESTDIR)$(PREFIX)/lib/
