#!/bin/bash
set -exuo

ADDRESS="127.0.0.1"
PORT=8888
FILENAME="test_file"
WORK_DIR=$(mktemp -d)
trap 'rm -r "$WORK_DIR"' EXIT

./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR"   &
SERVER_PID=$!

FILE_SIZE=$((512*256))
# Generate random file for upload
head -c$FILE_SIZE /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$FILENAME"_gen

TEST_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME"_gen | cut -d ' ' -f 1)

curl -T "$WORK_DIR"/"$FILENAME"_gen tftp://"$ADDRESS":"$PORT"/"$FILENAME"

trap 'rm -r "$WORK_DIR" && kill -9 $SERVER_PID' EXIT

UPLOADED_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME" | cut -d ' ' -f 1)

if [[ "$TEST_FILE_CHECKSUM" == "$UPLOADED_FILE_CHECKSUM" ]]; then
    exit 0
else
    exit 1
fi
