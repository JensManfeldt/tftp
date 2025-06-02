#!/bin/bash

unshare --net  bash -c 'ip link set lo up && $(pwd)"/test/download_file.sh"' &> /dev/null

if [ $? -eq 0 ]; then
    echo "Download file test passed"
else
    echo "Download file test failed"
fi

unshare --net  bash -c 'ip link set lo up && $(pwd)"/test/upload_file.sh"' &> /dev/null

if [ $? -eq 0 ]; then
    echo "Upload file test passed"
else
    echo "Upload file test failed"
fi

unshare --net  bash -c 'ip link set lo up && $(pwd)"/test/two_download_conns.sh"' &> /dev/null

if [ $? -eq 0 ]; then
    echo "Two download conns test passed"
fi
