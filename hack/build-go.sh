#!/bin/bash

set -ex

OUT_BIN=$1
OUT_DIR=$HMAKE_PROJECT_DIR/bin
shift

if [ -n "$GOOS$GOARCH" ]; then
    OUT_DIR=$OUTDIR/$GOOS/$GOARCH
fi

mkdir -p $OUT_DIR
CGO_ENABLED=0 go build -o $OUT_DIR/$OUT_BIN \
    -a -installsuffix netgo \
    -ldflags "$GO_LDFLAGS -extldflags -static" \
    $@
