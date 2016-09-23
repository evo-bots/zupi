#!/bin/bash
exec sim-ng vis -I $(dirname $BASH_SOURCE) -- "$@"
