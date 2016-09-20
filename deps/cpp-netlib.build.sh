#!/bin/bash
set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/cpp-netlib
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/cpp-netlib
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

CMAKE_OPTS=
if [ "$ARCH" == "armhf" ]; then
    CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_TOOLCHAIN_FILE=$HMAKE_PROJECT_DIR/common/cmake/armhf.toolchain.cmake"
fi

rm -fr $BLD_DIR
mkdir -p $BLD_DIR
cd $BLD_DIR

cmake $CMAKE_OPTS \
    -DBOOST_INCLUDEDIR=$OUT_DIR/include \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCPP-NETLIB_BUILD_TESTS=OFF \
    -DCPP-NETLIB_BUILD_EXAMPLES=OFF \
    -DCPP-NETLIB_ENABLE_HTTPS=OFF \
    $SRC_DIR

make $MAKE_OPTS
make install DESTDIR=./_install

mkdir -p $OUT_DIR
cp -rf _install/usr/local/* $OUT_DIR/
