#!/bin/bash
set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/dlib
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/dlib
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

CMAKE_OPTS=
case "$ARCH" in
    amd64)
        CMAKE_OPTS="$CMAKE_OPTS -DUSE_SSE4_INSTRUCTIONS=ON -DUSE_SSE2_INSTRUCTIONS=ON -DUSE_AVX_INSTRUCTIONS=ON"
        ;;
    armhf)
        CMAKE_OPTS="$CMAKE_OPTS -DDLIB_NO_GUI_SUPPORT=ON"
        CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_TOOLCHAIN_FILE=$HMAKE_PROJECT_DIR/common/cmake/armhf.toolchain.cmake"
        ;;
esac

rm -fr $BLD_DIR
mkdir -p $BLD_DIR
cd $BLD_DIR

export PKG_CONFIG_PATH=$OUT_DIR/lib/pkgconfig
export OPENBLAS_HOME=$OUT_DIR
cmake $CMAKE_OPTS \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_PREFIX_PATH=/usr/local \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DDLIB_USE_BLAS=ON \
    -DDLIB_USE_LAPACK=ON \
    $SRC_DIR

make $MAKE_OPTS
make install DESTDIR=./_install

mkdir -p $OUT_DIR
cp -rf _install/usr/local/* $OUT_DIR/
