#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/minit
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

TC_PREFIX=
case "$ARCH" in
    armhf) TC_PREFIX=arm-linux-gnueabihf- ;;
esac

mkdir -p $OUT_DIR/bin
${TC_PREFIX}gcc -Os -Wall -pedantic -std=gnu99 -s -static -o $OUT_DIR/bin/minit $SRC_DIR/minit.c
