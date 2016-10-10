#!/bin/bash

set -ex

. $(dirname $BASH_SOURCE)/functions.sh

ARCH="$1"
PROJECT_DIR="$2"
CMAKE_BUILD_TYPE="$3"

test -n "$ARCH"
test -n "$PROJECT_DIR" -a -d "$SRC_ROOT/$PROJECT_DIR"
test "$CMAKE_BUILD_TYPE" == "Debug" -o "$CMAKE_BUILD_TYPE" == "Release"

shift; shift; shift

BINS="$@"

CMAKE_OPTS=
case "$ARCH" in
    armhf)
        CMAKE_OPTS="$CMAKE_OPTS -DCMAKE_TOOLCHAIN_FILE=$SRC_ROOT/common/cmake/armhf.toolchain.cmake"
    ;;
esac

OUT_DIR=$OUT_BASE/$ARCH
BLD_DIR=$BLD_BASE/$ARCH/$PROJECT_DIR/$CMAKE_BUILD_TYPE
test -n "$NO_CLEAN" || rm -fr $BLD_DIR
mkdir -p $BLD_DIR
cd $BLD_DIR
cmake $CMAKE_OPTS \
    -DARCH=$ARCH \
    -DCMAKE_MODULE_PATH=$SRC_ROOT/common/cmake \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$OUT_DIR \
    "$SRC_ROOT/$PROJECT_DIR"
make VERBOSE=1 $MAKE_OPTS
mkdir -p "$OUT_DIR/lib" "$OUT_DIR/bin"
for bin in $BINS; do
    if [ "${bin##lib}" != "$bin" -a "${bin%%.a}" != "$bin" ]; then
        cp -rf "$BLD_DIR/$bin" "$OUT_DIR/lib/"
    else
        cp -rf "$BLD_DIR/$bin" "$OUT_DIR/bin/"
    fi
done
