#!/bin/bash

pushd ..
make libs
popd
rm alinput_test
make alinput_test
./alinput_test <t
