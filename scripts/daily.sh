#!/bin/bash
# open a emacs on a new text file to track daily activity

usage()
{
    head -n 2
}

YEAR=$(date +"%Y")
TODAYDIR=$(date +"%d%m")
DAILYDIR=activity

if [[ ! -d ${DAILYDIR} ]]
then
    echo "[ERROR] Expect ${DAILYDIR} directory" >&2
    usage
    exit 1
fi

pushd ${DAILYDIR} >/dev/null

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
    usage
fi

popd >/dev/null
