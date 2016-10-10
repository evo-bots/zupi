#!/bin/bash

set -ex

. $(dirname $BASH_SOURCE)/functions.sh

ARCH=$1
PKG=$2
test -n "$ARCH"
test -n "$PKG"

NPM_PKG=$(find $OUT_BASE -name "$PKG-*.tgz")
test -f "$NPM_PKG"

BLD_DIR=$BLD_BASE/$ARCH/$PKG
OUT_DIR=$OUT_BASE/$ARCH

case "$ARCH" in
    amd64)
        # do nothing
    ;;
    armhf)
        export CC=arm-linux-gnueabihf-gcc
        export CXX=arm-linux-gnueabihf-g++
        export LINK=arm-linux-gnueabihf-g++
        export npm_config_arch=$ARCH
    ;;
    *)
        echo Unknown ARCH=$ARCH >&2
        exit 1
    ;;
esac

rm -fr $BLD_DIR
mkdir -p $BLD_DIR $OUT_DIR
tar -C $BLD_DIR --strip-components=1 -zxf $NPM_PKG
cd $BLD_DIR
npm install --build-from-source --production
tar -zcf $OUT_DIR/$PKG.tar.gz *
