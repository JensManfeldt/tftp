#!/bin/bash

set pipefail -exuo

ADDRESS="127.0.0.1"
PORT=8888
WORK_DIR=$(mktemp -d)
trap 'rm -r "$WORK_DIR"' EXIT


touch "$WORK_DIR"/test_file.txt
echo "This is a test_file" > "$WORK_DIR"/test_file

./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR"
