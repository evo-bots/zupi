#!/bin/bash
set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/openblas
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/openblas
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

OPTS="NO_SHARED=1"
case "$ARCH" in
    amd64)
        OPTS="$OPTS TARGET=SANDYBRIDGE BINARY=64"
        ;;
    armhf)
        OPTS="$OPTS TARGET=ARMV7 BINARY=32 HOSTCC=gcc CC=arm-linux-gnueabihf-gcc FC=arm-linux-gnueabihf-gfortran"
        ;;
esac

rm -fr $BLD_DIR
mkdir -p $(dirname $BLD_DIR)
cp -rf $SRC_DIR $BLD_DIR
cd $BLD_DIR

make $MAKE_OPTS $OPTS
make install $MAKE_OPTS $OPTS PREFIX=$OUT_DIR/
