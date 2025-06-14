
#!/bin/bash
set pipefail -exuo
ADDRESS="127.0.0.1"
PORT=8888
FILENAME="test_file"
WORK_DIR=$(mktemp -d)

head -c4096 /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$FILENAME"
TEST_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME" | cut -d ' ' -f 1)

./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR"  &
SERVER_PID=$!

./build/client -a "$ADDRESS" -p $PORT -f "$WORK_DIR"/"$FILENAME" -r


trap 'rm -r "$WORK_DIR" && kill -9 $SERVER_PID' EXIT
if [[ $? -eq 0 ]] then
    echo "re-transmission rrq test success"
    exit 0
else
    echo "re-transmission rrq test failed"
    exit 1
fi
