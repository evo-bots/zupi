#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/zlib
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/zlib
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR

case "$ARCH" in
    armhf) export CROSS_PREFIX=arm-linux-gnueabihf- ;;
esac

./configure --static
make $MAKE_OPTS
make install DESTDIR=./_install
mkdir -p $OUT_DIR
cp -rf _install/usr/local/* $OUT_DIR/
