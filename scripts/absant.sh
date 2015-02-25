#!/bin/bash

buildscript=$1

if [[ -e $buildscript ]]
then
    # protect ${ variables.
    sed 's/\${/\\\${/g' build.xml
else
    echo "[ERROR] this $0 tool expect an initial argument with buildscript" >&2
fi
    
