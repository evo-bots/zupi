#!/bin/bash

set -ex

test -n "$HMAKE_PROJECT_DIR"
test -d "$HMAKE_PROJECT_DIR"

PROJECT_DIR="$1"
CMAKE_BUILD_TYPE="$2"

test -n "$PROJECT_DIR" -a -d "$HMAKE_PROJECT_DIR/$PROJECT_DIR"
test "$CMAKE_BUILD_TYPE" == "Debug" -o "$CMAKE_BUILD_TYPE" == "Release"

shift; shift;

BINS="$@"

rm -fr "$HMAKE_PROJECT_DIR/_build/build/$PROJECT_DIR/$CMAKE_BUILD_TYPE"
mkdir -p "$HMAKE_PROJECT_DIR/_build/build/$PROJECT_DIR/$CMAKE_BUILD_TYPE"
cd "$HMAKE_PROJECT_DIR/_build/build/$PROJECT_DIR/$CMAKE_BUILD_TYPE"
cmake \
    -DCMAKE_MODULE_PATH=$HMAKE_PROJECT_DIR/common/cmake \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$HMAKE_PROJECT_DIR/_build/ \
    "$HMAKE_PROJECT_DIR/$PROJECT_DIR"
make VERBOSE=1 $MAKE_OPTS
for bin in $BINS; do
    if [ "${bin##lib}" != "$bin" -a "${bin%%.a}" != "$bin" ]; then
        cp -rf "$HMAKE_PROJECT_DIR/_build/build/$PROJECT_DIR/$CMAKE_BUILD_TYPE/$bin" \
            "$HMAKE_PROJECT_DIR/_build/lib/"
    else
        cp -rf "$HMAKE_PROJECT_DIR/_build/build/$PROJECT_DIR/$CMAKE_BUILD_TYPE/$bin" \
            "$HMAKE_PROJECT_DIR/bin/"
    fi
done
