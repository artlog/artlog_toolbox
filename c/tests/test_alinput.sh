#!/bin/bash

pushd ..
make libs
popd
rm alinput_test
make alinput_test
echo "1234567890ABCDEFGHIJKLMNOPQ" | ./alinput_test 
