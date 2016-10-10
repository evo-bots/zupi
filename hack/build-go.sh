#!/bin/bash

set -ex

. $(dirname $BASH_SOURCE)/functions.sh

ARCH=$1
OUT_BIN=$2
test -n "$ARCH"
test -n "$OUT_BIN"
OUT_DIR=$OUT_BASE/$ARCH

shift; shift

LDFLAGS=
if [ -n "$RELEASE" ]; then
    case "$RELEASE" in
        y|yes|final) LDFLAGS="$LDFLAGS -X main.VersionSuffix=" ;;
        *) LDFLAGS="$LDFLAGS -X main.VersionSuffix=-$RELEASE" ;;
    esac
else
    suffix=$(git log -1 --format=%h || true)
    if [ -n "$suffix" ]; then
        test -z "$(git status --porcelain || true)" || suffix="${suffix}+"
        LDFLAGS="$LDFLAGS -X main.VersionSuffix=-g${suffix}"
    fi
fi

case "$ARCH" in
    amd64)
        export GOARCH=$ARCH
    ;;
    arm|armel)
        export GOARCH=arm
    ;;
    armhf)
        export GOARCH=arm
        export GOARM=7
    ;;
    arm64)
        export GOARCH=arm64
    ;;
    *)
        echo Unknown ARCH=$ARCH >&2
        exit 1
    ;;
esac

mkdir -p $OUT_DIR/bin
CGO_ENABLED=0 go build -o $OUT_DIR/bin/$OUT_BIN \
    -a -installsuffix netgo \
    -ldflags "$LDFLAGS $GO_LDFLAGS -extldflags -static" \
    $@
