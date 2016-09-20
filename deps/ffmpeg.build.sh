#!/bin/bash
set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/ffmpeg
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/ffmpeg
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

CONFIG_OPTS=
case "$ARCH" in
    armhf)
        CONFIG_OPTS="$CONFIG_OPTS --enable-cross-compile --cross-prefix=arm-linux-gnueabihf- --arch=armhf --target-os=linux"
    ;;
esac

rm -fr $BLD_DIR
mkdir -p $BLD_DIR
cd $BLD_DIR

$SRC_DIR/configure $CONFIG_OPTS --disable-doc --disable-shared
make $MAKE_OPTS
make install DESTDIR=./_install
mkdir -p $OUT_DIR
cp -rf _install/usr/local/* $OUT_DIR/
rm -f $OUT_DIR/lib/libavresample.a
ln -s libswresample.a $OUT_DIR/lib/libavresample.a
