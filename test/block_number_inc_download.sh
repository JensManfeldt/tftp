#!/bin/bash
set -exuo

ADDRESS="127.0.0.1"
PORT=8888
FILENAME="test_file"
WORK_DIR=$(mktemp -d)
trap 'rm -r "$WORK_DIR"' EXIT

./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR"  &
SERVER_PID=$!

# Should require two bytes to represent
FILE_SIZE=$((512*256))

head -c$FILE_SIZE /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$FILENAME"
TEST_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME" | cut -d ' ' -f 1)

curl -o "$WORK_DIR"/test_download tftp://"$ADDRESS":"$PORT"/"$FILENAME"

trap 'rm -r "$WORK_DIR" && kill -9 $SERVER_PID' EXIT

DOWNLOADED_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/test_download | cut -d ' ' -f 1)

if [[ "$TEST_FILE_CHECKSUM" == "$DOWNLOADED_FILE_CHECKSUM" ]]; then
    exit 0
else
    exit 1
fi
