#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/fido
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/fido
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR
find src -name '*.cpp' -exec sed -i -r 's/std::cout/std::cerr/g' '{}' ';'

case "$ARCH" in
    armhf) export CXX=arm-linux-gnueabihf-g++ ;;
esac

make $MAKE_OPTS install DESTDIR=./_install
mkdir -p $OUT_DIR/include $OUT_DIR/lib
cp -rf _install/usr/local/include/Fido $OUT_DIR/include/
cp -f _install/usr/local/lib/fido.a $OUT_DIR/lib/libfido.a
