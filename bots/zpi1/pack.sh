#!/bin/bash

. ../../hack/functions.sh

set -ex

ARCH=$1
test -n "$ARCH"

BLD_DIR=$BLD_BASE/$ARCH/zpi1
OUT_DIR=$OUT_BASE/$ARCH
DEST_DIR=$BLD_DIR/opt/zupi/zpi1

rm -fr $BLD_DIR

mkdir -p $BLD_DIR/etc/systemd/system
mkdir -p $DEST_DIR/bin
mkdir -p $DEST_DIR/share

cp -f $OUT_DIR/bin/zpi1-brain $DEST_DIR/bin/
cp -f $OUT_DIR/bin/zpi1-vision $DEST_DIR/bin/
cp -rf $OUT_DIR/share/OpenCV $DEST_DIR/share/

mksvc() {
    local name=$1
    shift;
    cat >$BLD_DIR/etc/systemd/system/$name.service <<EOF
[Unit]
Description=$name
After=network.target

[Service]
ExecStart=$@
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF
}

install_brain() {
    cat >$DEST_DIR/bin/brain <<EOF
#!/bin/sh
export PATH=/opt/zupi/zpi1/bin:\$PATH
export LOGXI=*=DBG
exec /opt/zupi/zpi1/bin/zpi1-brain brain
EOF
    chmod a+rx $DEST_DIR/bin/brain
    mksvc zpi1-brain /opt/zupi/zpi1/bin/brain
}

install_robot() {
    mkdir -p $DEST_DIR/share/robot
    tar -C "$DEST_DIR/share/robot" -zxf $OUT_DIR/zpi1-robot.tar.gz
    cat >$DEST_DIR/share/robot/run.sh <<EOF
#!/bin/sh
cd /opt/zupi/zpi1/share/robot
export DEBUG=zpi1.*
exec node app.js "\$@"
EOF
    chmod a+rx $DEST_DIR/share/robot/run.sh

    cat >$DEST_DIR/bin/robot <<EOF
#!/bin/sh
DEBUG=zpi1.* /opt/zupi/zpi1/share/robot/run.sh reset
export LOGXI=*=DBG
exec /opt/zupi/zpi1/bin/zpi1-brain conn -- /opt/zupi/zpi1/share/robot/run.sh connect '{{.host}}' -p '{{.port}}'
EOF
    chmod a+rx $DEST_DIR/bin/robot

    mksvc zpi1-robot /opt/zupi/zpi1/bin/robot
}

install_brain
install_robot

tar --posix --owner=0 --group=0 --no-acls --no-xattrs \
    -C $BLD_DIR -czf $OUT_DIR/zpi1.tar.gz etc opt
