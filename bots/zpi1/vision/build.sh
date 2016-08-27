#!/bin/bash

set -ex

test -n "$HMAKE_PROJECT_DIR"
test -d "$HMAKE_PROJECT_DIR"

CMAKE_BUILD_TYPE="$1"
test "$CMAKE_BUILD_TYPE" == "Debug" -o "$CMAKE_BUILD_TYPE" == "Release"

rm -fr "$HMAKE_PROJECT_DIR/_build/build/vision/$CMAKE_BUILD_TYPE"
mkdir -p "$HMAKE_PROJECT_DIR/_build/build/vision/$CMAKE_BUILD_TYPE"
cd "$HMAKE_PROJECT_DIR/_build/build/vision/$CMAKE_BUILD_TYPE"
cmake \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    -DCMAKE_PREFIX_PATH=$HMAKE_PROJECT_DIR/_build/ \
    ../../../../bots/zpi1/vision
make VERBOSE=1 $MAKE_OPTS
