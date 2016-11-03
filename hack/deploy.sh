#!/bin/bash

set -ex

. $(dirname $BASH_SOURCE)/functions.sh

ARCH="$1"
PKG="$2"
test -n "$HOST" || HOST="$3"
shift; shift; shift;

if [ "${HOST%%@*}" != "root" ]; then
    SUDO=sudo
fi

remote_sudo() {
    ssh -o StrictHostKeyChecking=no "$HOST" $SUDO "$@"
}

cat $OUT_BASE/$ARCH/$PKG.tar.gz | remote_sudo tar -C / -zx

remote_sudo systemctl daemon-reload

for svc in $@; do
    remote_sudo systemctl enable $svc
    remote_sudo systemctl restart $svc
done
