#!/bin/bash

test_script=./test1.sh

for project in abnf base64 cbor hashtree json
do
    pushd $project
    if [[ -f $test_script ]]
    then
	$test_script
    else
	make clean
	make build
    fi    
    popd
done
