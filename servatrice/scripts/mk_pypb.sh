#!/bin/bash

SRC_DIR=../../common/pb/
DST_DIR=./pypb

rm -rf "$DST_DIR"
mkdir -p "$DST_DIR"
protoc -I=$SRC_DIR --python_out=$DST_DIR $SRC_DIR/*.proto
touch "$DST_DIR/__init__.py"

