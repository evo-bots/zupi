#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/openssl
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/openssl
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR

CONFIG_OPTS=
case "$ARCH" in
    amd64) CONFIG_OPTS=linux-x86_64 ;;
    armhf) CONFIG_OPTS="linux-armv4 -march=armv7-a --cross-compile-prefix=arm-linux-gnueabihf-" ;;
esac

./Configure $CONFIG_OPTS \
    --with-zlib-include=$OUT_DIR/include \
    --with-zlib-lib=$OUT_DIR/lib \
    no-shared \
    no-dso \
    threads \
    zlib
make
make install DESTDIR=./_install
mkdir -p $OUT_DIR/lib
cp -rf _install/usr/local/include $OUT_DIR/
cp -rf _install/usr/local/lib/pkgconfig $OUT_DIR/lib/
cp -f _install/usr/local/lib/lib*.a $OUT_DIR/lib/
cp -rf _install/usr/local/bin $OUT_DIR/
