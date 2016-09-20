#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/bzip2
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/bzip2
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR

TC_PREFIX=
case "$ARCH" in
    armhf) TC_PREFIX=arm-linux-gnueabihf- ;;
esac

if [ -n "$TC_PREFIX" ]; then
    sed -i -r "s/^CC=.+$/CC=${TC_PREFIX}gcc/g" Makefile
    sed -i -r "s/^AR=.+$/AR=${TC_PREFIX}ar/g" Makefile
    sed -i -r "s/^RANLIB=.+$/RANLIB=${TC_PREFIX}ranlib/g" Makefile
fi

make $MAKE_OPTS libbz2.a
mkdir -p $OUT_DIR/include $OUT_DIR/lib
cp -f bzlib.h $OUT_DIR/include/
cp -f libbz2.a $OUT_DIR/lib/
