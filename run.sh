#!/bin/bash

set pipefail -exuo

ADDRESS="127.0.0.1"
PORT=8888
WORK_DIR=$(mktemp -d)

./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR"
SERVER_PID=$!

trap 'rm -r "$WORK_DIR" && kill -9 $SERVER_PID' EXIT
