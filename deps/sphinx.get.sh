#!/bin/bash

set -ex

SPHINX_BASE_URL='http://downloads.sourceforge.net/project/cmusphinx/sphinxbase/5prealpha/sphinxbase-5prealpha.tar.gz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fcmusphinx%2Ffiles%2Fsphinxbase%2F5prealpha%2F&ts=1476164059&use_mirror=heanet'
POCKET_SPHINX_URL='http://downloads.sourceforge.net/project/cmusphinx/pocketsphinx/5prealpha/pocketsphinx-5prealpha.tar.gz?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fcmusphinx%2Ffiles%2Fpocketsphinx%2F5prealpha%2F&ts=1476165023&use_mirror=heanet'

SRC_DIR=$HMAKE_PROJECT_DIR/_build/src/sphinx
BASE_DIR=$SRC_DIR/sphinxbase
POCKET_DIR=$SRC_DIR/pocketsphinx

rm -fr $SRC_DIR
mkdir -p $BASE_DIR
mkdir -p $POCKET_DIR

curl -sSL "$SPHINX_BASE_URL" | tar -C $BASE_DIR --strip-components=1 -zx
curl -sSL "$POCKET_SPHINX_URL" | tar -C $POCKET_DIR --strip-components=1 -zx
