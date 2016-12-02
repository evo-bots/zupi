#!/bin/bash

PROJECT="$1"
URL="$2"

test -n "$PROJECT"
test -n "$URL"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/$PROJECT

PKG_SUFFIX="${URL##*.}"
TAR=tar
TAR_OPT=

case "$PKG_SUFFIX" in
    tgz|gz) TAR_OPT=-zx ;;
    bz2) TAR_OPT=-jx ;;
    xz) TAR_OPT=-Jx ;;
    zip)
        TAR=bsdtar
        TAR_OPT=-xf-
        ;;
    *)
        echo Unknown package format $PKG_SUFFIX >&2
        exit 1
    ;;
esac

test -n "$TAR_NO_STRIP_COMPONENTS" || \
    TAR_OPT="--strip-components=${TAR_STRIP_COMPONENTS:-1} $TAR_OPT"

rm -fr "$SRC_DIR"
mkdir -p "$SRC_DIR"
curl -sSL "$URL" | $TAR -C "$SRC_DIR" $TAR_OPT
