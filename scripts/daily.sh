#!/bin/bash

YEAR=$(date +"%Y")
TODAYDIR=$(date +"%d%m")

if [[ ! -d activity ]]
then
    echo "[ERROR] Expect activity directory" >&2
    exit 1
fi

pushd activity >/dev/null

if [[ -d $YEAR ]]
then
    pushd $YEAR
    [[ -d $TODAYDIR ]] || mkdir $TODAYDIR
    if [[ -d $TODAYDIR ]]
    then
	pushd $TODAYDIR
	emacs whatido.txt&
	popd
    fi
    popd
else
    echo "[ERROR] Missing directory $YEAR in $(pwd)" >&2
fi

popd >/dev/null
