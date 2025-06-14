
#!/bin/bash
set pipefail -exuo

ADDRESS="127.0.0.1"
PORT=8888
FILENAME="test_file"
WORK_DIR=$(mktemp -d)

# start server
./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR" -c 2 &
SERVER_PID=$!

TEST_FILE_1="$FILENAME"_1
head -c16K /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$TEST_FILE_1"
TEST_FILE_1_CHECKSUM=$(sha256sum "$WORK_DIR"/"$TEST_FILE_1" | cut -d ' ' -f 1)
echo "$TEST_FILE_1_CHECKSUM"

TEST_FILE_2="$FILENAME"_2
head -c16K /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$TEST_FILE_2"
TEST_FILE_2_CHECKSUM=$(sha256sum "$WORK_DIR"/"$TEST_FILE_2" | cut -d ' ' -f 1)
echo "$TEST_FILE_2_CHECKSUM"


curl -o "$WORK_DIR"/test_download_1 tftp://"$ADDRESS":"$PORT"/"$TEST_FILE_1" &
P1=$!
curl -o "$WORK_DIR"/test_download_2 tftp://"$ADDRESS":"$PORT"/"$TEST_FILE_2" &
P2=$!

wait "$P1" "$P2"

DOWNLOADED_FILE_1_CHECKSUM=$(sha256sum "$WORK_DIR"/test_download_1 | cut -d ' ' -f 1)
echo "$DOWNLOADED_FILE_1_CHECKSUM"
DOWNLOADED_FILE_2_CHECKSUM=$(sha256sum "$WORK_DIR"/test_download_2 | cut -d ' ' -f 1)
echo "$DOWNLOADED_FILE_2_CHECKSUM"

trap 'rm -r "$WORK_DIR" && kill -9 $SERVER_PID' EXIT

if [[ "$TEST_FILE_1_CHECKSUM" == "$DOWNLOADED_FILE_1_CHECKSUM" && "$TEST_FILE_2_CHECKSUM" == "$DOWNLOADED_FILE_2_CHECKSUM" ]]; then
    exit 0
else
    exit 1
fi
