#!/bin/sh

set -xe

CXX=${CXX:="c++"}
CFLAGS=${CFLAGS:="-Wall -Wextra -pedantic -pipe -ggdb"}

$CXX $CFLAGS -o build main.cpp
