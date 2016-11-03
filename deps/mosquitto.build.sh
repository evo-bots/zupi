#!/bin/bash

set -ex

ARCH=$1
test -n "$ARCH"

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/mosquitto
BLD_BASE=$HMAKE_PROJECT_DIR/_build/$ARCH
BLD_DIR=$BLD_BASE/mosquitto
OUT_DIR=$HMAKE_PROJECT_DIR/out/$ARCH

rm -fr $BLD_DIR
mkdir -p $BLD_BASE
cp -rf $SRC_DIR $BLD_BASE/
cd $BLD_DIR

case "$ARCH" in
    armhf) export CROSS_COMPILE=arm-linux-gnueabihf- ;;
esac

CFLAGS="$CFLAGS -I$OUT_DIR/include"
CFLAGS="$CFLAGS -DWITH_TLS -DWITH_TLS_PSK -DWITH_THREADING -DWITH_SRV -DWITH_SOCKS"
LDFLAGS="$LDFLAGS -L$OUT_DIR/lib -lcares -lwebsockets -lssl -lcrypto -lz -lrt -lpthread"
FOR_BROKER="-DWITH_BRIDGE -DWITH_PERSISTENCE -DWITH_MEMORY_TRACKING -DWITH_SYS_TREE -DWITH_WEBSOCKETS -DWITH_EC"

make -C lib install DESTDIR=$BLD_DIR/_install CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS"
${CROSS_COMPILE}g++ $CFLAGS -Ilib/cpp -Ilib -c -o lib/cpp/mosquittopp.o lib/cpp/mosquittopp.cpp
${CROSS_COMPILE}ar cr _install/usr/local/lib/libmosquittopp.a lib/cpp/mosquittopp.o
make -C src install DESTDIR=$BLD_DIR/_install WITH_UUID=no CFLAGS="$CFLAGS $FOR_BROKER" LDFLAGS="$LDFLAGS"
${CROSS_COMPILE}gcc -Iclient -Ilib -O2 -s '-DVERSION="1.4.10"' \
    -o _install/usr/local/bin/mosquitto_sub client/sub_client.c client/client_shared.c -Llib -lmosquitto $LDFLAGS
${CROSS_COMPILE}gcc -Iclient -Ilib -O2 -s '-DVERSION="1.4.10"' \
    -o _install/usr/local/bin/mosquitto_pub client/pub_client.c client/client_shared.c -Llib -lmosquitto $LDFLAGS
mkdir -p $OUT_DIR/lib
cp -rf _install/usr/local/include $OUT_DIR/
cp -f lib/libmosquitto.a $OUT_DIR/lib
cp -f _install/usr/local/lib/libmosquittopp.a $OUT_DIR/lib
cp -rf _install/usr/local/bin $OUT_DIR/
cp -f _install/usr/local/sbin/mosquitto $OUT_DIR/bin/
