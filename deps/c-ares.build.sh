#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/c-ares
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/c-ares
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR

CONFIG_OPTS=
case "$ARCH" in
    armhf) CONFIG_OPTS=--host=arm-linux-gnueabihf ;;
esac

./configure $CONFIG_OPTS \
    --disable-shared --disable-debug --disable-silent-rules \
    --enable-optimize --with-pic
make
make install DESTDIR=$BLD_DIR/_install
mkdir -p $OUT_DIR
cp -rf _install/usr/local/include $OUT_DIR/
cp -rf _install/usr/local/lib $OUT_DIR/
