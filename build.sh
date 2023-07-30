#!/bin/sh

set -xe

CXX=${CXX:="c++"}
CFLAGS=${CFLAGS:="-Wall -Wextra -pedantic -pipe -ggdb $(pkg-config --cflags openssl)"}
LIBS=$(pkg-config --libs openssl)

$CXX $CFLAGS -o build main.cpp $LIBS
