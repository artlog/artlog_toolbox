#!/bin/bash

for project in abnf base64 cbor hashtree json
do
    pushd $project
    ./test1.sh
    popd
done
