#!/bin/bash

SCRIPT=$(readlink -f "$0")
BASEDIR=$(dirname "$SCRIPT")

cd $BASEDIR

git clone "https://github.com/warmcat/libwebsockets.git"

cd libwebsockets && mkdir build && cd build

CFLAGS="-I$1" LDFLAGS="-L$2" cmake .. -DCMAKE_INSTALL_PREFIX=${prefix} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TARGET_TOOLCHAIN} -DCMAKE_BUILD_TYPE=Release -DZLIB_LIBRARY=$2/libz.a -DZLIB_INCLUDE_DIR=$1 -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_LINK_TESTAPPS_DYNAMIC=ON && make

cd $BASEDIR && make

