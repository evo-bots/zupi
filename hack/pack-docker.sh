#!/bin/bash

set -ex

PACK=$1
ARCH=$2
test -n "$PACK" -a -n "$ARCH"

SRCDIR=$(pwd)
DOCKERFILE_SRC=$SRCDIR/Dockerfile
OUTDIR=$HMAKE_PROJECT_DIR/out/$ARCH
DOCKERFILE=$OUTDIR/$PACK.Dockerfile

if ! [ -f "$DOCKERFILE_SRC" ]; then
    DOCKERFILE_SRC=${DOCKERFILE_SRC}.$ARCH
fi

cp -f $DOCKERFILE_SRC $DOCKERFILE
if [ -d "$SRCDIR/assets" ]; then
    rm -fr $OUTDIR/assets/$PACK
    mkdir -p $OUTDIR/assets
    cp -rf "$SRCDIR/assets" $OUTDIR/assets/$PACK
fi

cd $OUTDIR
docker build -t zupi-pack-$PACK-$ARCH:build --build-arg ARCH=$ARCH -f $PACK.Dockerfile .
mkdir -p $OUTDIR/docker
docker save zupi-pack-$PACK-$ARCH:build | gzip -9 >docker/$PACK-$ARCH.tar.gz
