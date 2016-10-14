#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/sphinx
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/sphinx
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $(dirname $BLD_DIR)
cp -rf $SRC_DIR $BLD_DIR

HOST_OPT=
case "$ARCH" in
    armhf) HOST_OPT="--host=arm-linux-gnueabihf" ;;
esac

cd $BLD_DIR/sphinxbase
./configure $HOST_OPT --disable-shared --enable-static --without-python
make $MAKE_OPTS
make install DESTDIR=`pwd`/_install

cd $BLD_DIR/pocketsphinx
./configure $HOST_OPT --disable-shared --enable-static --without-python
make $MAKE_OPTS
make install DESTDIR=`pwd`/_install

mkdir -p $OUT_DIR
cp -rf $BLD_DIR/sphinxbase/_install/usr/local/* $OUT_DIR/
cp -rf $BLD_DIR/pocketsphinx/_install/usr/local/* $OUT_DIR/
