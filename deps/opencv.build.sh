#!/bin/bash
set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/opencv
BLD_DIR=$HMAKE_PROJECT_DIR/_build/$ARCH/opencv
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

CMAKE_OPTS=
if [ "$ARCH" == "armhf" ]; then
    CMAKE_OPTS="$CMAKE_OPTS -DENABLE_VFPV3=ON -DENABLE_NEON=ON -DWITH_TBB=ON -DBUILD_TBB=ON"
    CMAKE_OPTS="$CMAKE_OPTS -DGCC_COMPILER_VERSION=4.9 -DCMAKE_TOOLCHAIN_FILE=$SRC_DIR/platforms/linux/arm-gnueabi.toolchain.cmake"
fi

rm -fr $BLD_DIR
mkdir -p $BLD_DIR
cd $BLD_DIR

export PKG_CONFIG_PATH=$OUT_DIR/lib/pkgconfig
cmake $CMAKE_OPTS \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_DOCS=OFF \
    -DBUILD_JPEG=ON \
    -DBUILD_PNG=ON \
    -DBUILD_TIFF=ON \
    -DBUILD_ZLIB=ON \
    -DBUILD_JASPER=ON \
    -DBUILD_OPENEXR=ON \
    -DFFMPEG_INCLUDE_DIR=$OUT_DIR/include \
    -DFFMPEG_LIB_DIR=$OUT_DIR/lib \
    -DFFMPEG_CODEC_LIB=$OUT_DIR/lib/libavcodec.a \
    -DFFMPEG_FORMAT_LIB=$OUT_DIR/lib/libavformat.a \
    -DFFMPEG_UTIL_LIB=$OUT_DIR/lib/libavutil.a \
    -DFFMPEG_SWSCALE_LIB=$OUT_DIR/lib/libswscale.a \
    -DFFMPEG_RESAMPLE_LIB=$OUT_DIR/lib/libavresample.a \
    -DCMAKE_PREFIX_PATH=/usr/local \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    $SRC_DIR

make $MAKE_OPTS
make install DESTDIR=./_install

mkdir -p $OUT_DIR
cp -rf _install/usr/local/* $OUT_DIR/
