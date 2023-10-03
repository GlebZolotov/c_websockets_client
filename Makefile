NAME_END = so
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	NAME_END = dylib
endif

all: websockets_client.$(NAME_END)

ws.o: websockets.c libwebsockets/build/lib/libwebsockets.$(NAME_END)
	gcc -c -Wall $< -o $@ -Ilibwebsockets/build/include -fPIC
websockets_client.$(NAME_END): ws.o libwebsockets/build/lib/libwebsockets.$(NAME_END)
	gcc $^ -o $@ -Ilibwebsockets/build/include libwebsockets/build/lib/libwebsockets.$(NAME_END) -fPIC -shared
test.o: test.c websockets_client.$(NAME_END)
	gcc -c test.c -o test.o
test: test.o websockets_client.$(NAME_END)
	gcc test.o websockets_client.$(NAME_END) -o test

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install: websockets_client.so libwebsockets/build/lib/libwebsockets.$(NAME_END)
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 websockets_client.$(NAME_END) $(DESTDIR)$(PREFIX)/lib/
	install -m 644 libwebsockets/build/lib/libwebsockets.$(NAME_END) $(DESTDIR)$(PREFIX)/lib/
