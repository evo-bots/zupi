#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/julius
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/julius
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $(dirname $BLD_DIR)
cp -rf $SRC_DIR $BLD_DIR

HOST_OPT=
case "$ARCH" in
    armhf) HOST_OPT="--host=arm-linux-gnueabihf" ;;
esac

cd $BLD_DIR
patch < $HMAKE_PROJECT_DIR/deps/julius.patch
LDFLAGS=-L$OUT_DIR/lib \
CFLAGS=-I$OUT_DIR/include \
./configure $HOST_OPT \
    --prefix=$OUT_DIR/ \
    --enable-words-int \
    --enable-msd \
    --enable-gmm-vad \
    --enable-decoder-vad \
    --enable-power-reject \
    --enable-setup=standard \
    --enable-factor2 \
    --enable-wpair \
    --enable-wpair-nlimit \
    --enable-word-graph
make $MAKE_OPTS
make install
