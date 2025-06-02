
#!/bin/bash
set pipefail -exuo

ADDRESS="127.0.0.1"
PORT=8888
FILENAME="test_file"
WORK_DIR=$(mktemp -d)
trap 'rm -r "$WORK_DIR"' EXIT

# start server
./build/main -a "$ADDRESS" -p $PORT -d "$WORK_DIR" &>/dev/null &

# Generate random file for upload
head -c4096 /dev/urandom | hexdump '-e"%x"' > "$WORK_DIR"/"$FILENAME"_gen

TEST_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME"_gen | cut -d ' ' -f 1)

curl -T "$WORK_DIR"/"$FILENAME"_gen tftp://"$ADDRESS":"$PORT"/"$FILENAME" &> /dev/null

UPLOADED_FILE_CHECKSUM=$(sha256sum "$WORK_DIR"/"$FILENAME" | cut -d ' ' -f 1)


if [[ "$TEST_FILE_CHECKSUM" == "$UPLOADED_FILE_CHECKSUM" ]]; then
    exit 0
else
    exit 1
fi
