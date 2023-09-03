# c_websockets_client
Tiny wrapper for libwebsockets library with simple interface

Interface is in `websockets.h`. Memory in `message` argument in `ws_send` function copy inside this function, 
thus you can make what you want with this memory after function return. Data in `ws_recv` returning value refers 
to a static internal library memory, and library admin it (please don't free it). 

For using library you need `bash`, `git`, `gcc` and `cmake`.

For build this library just run `build.sh` script. It clones `libwebsockets`, builds it with some `cmake` options
and builds shared library `websockets_client.so`. I use this shared library in Julia, and there is no problem with 
header files, but if you want to use library in C, please be carefully with headers and copy its to correct place
(or use `-I<path_to_headers>` and `-L<path_to_libraries>` flags in `gcc` commands for building and linking). 