#!/bin/bash

SCRIPT=$(readlink -f "$0")
BASEDIR=$(dirname "$SCRIPT")

cd $BASEDIR

git clone "https://github.com/warmcat/libwebsockets.git"

cd libwebsockets && mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Release -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_LINK_TESTAPPS_DYNAMIC=ON && make

cd $BASEDIR && make