#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/daemon
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/daemon
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR
sed -i -r 's/^((CC|AR|RANLIB)\s*:=\s*)/\1$(CROSS_COMPILE)/g' Makefile

case "$ARCH" in
    armhf)
        export CROSS_COMPILE=arm-linux-gnueabihf-
        ;;
esac

export CCFLAGS=-fPIC
export DAEMON_LDFLAGS=-static
make
mkdir -p $OUT_DIR/bin
${CROSS_COMPILE}strip -g -o $OUT_DIR/bin/daemon daemon
chmod a+rx $OUT_DIR/bin/daemon
