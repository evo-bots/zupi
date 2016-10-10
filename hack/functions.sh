# determine project root

SRC_ROOT=$HMAKE_PROJECT_DIR
if [ -z "$SRC_ROOT" ]; then
    SRC_ROOT="$(cd $(dirname $BASH_SOURCE)/..; pwd)"
fi

OUT_BASE="$SRC_ROOT/out"
BLD_BASE="$SRC_ROOT/_build"
