#!/bin/sh

set -xe

CXX=${CXX:="c++"}
CFLAGS=${CFLAGS:="-Wall -Wextra -Wno-deprecated-declarations -pedantic -pipe -ggdb $(pkg-config --cflags openssl)"}
LIBS=$(pkg-config --libs openssl)

$CXX $CFLAGS -o build main.cpp $LIBS
