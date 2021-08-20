#!/bin/bash
#
# open a emacs on a new text file to track daily activity
# h|-h|-help|help to get usage
#

usage()
{
    tail -n+2 $THIS_SCRIPT | head -n 3 
}

THIS_SCRIPT=$0
YEAR=$(date +"%Y")
TODAYDIR=$(date +"%d%m")
DAILYDIR=activity

while [[ $# > 0 ]]
do
    case $1 in
	create)
	    create=1
	    ;;
	h|-h|-help|help)
	    usage
	    exit 1
	    ;;
    esac
    shift
done


if [[ ! -d ${DAILYDIR} ]]
then
    if [[ $create == 1 ]]
    then
	mkdir -p ${DAILYDIR}
    else
	echo "[ERROR] Missing expected '${DAILYDIR}' directory or use 'create'" >&2
	usage
	exit 1
    fi
fi

pushd ${DAILYDIR} >/dev/null

if [[ ! -d $YEAR ]]
then
    if [[ $create == 1 ]]
    then
	mkdir $YEAR
    else
	echo "[ERROR] Missing directory $YEAR in $(pwd)" >&2
	usage
	exit 1
    fi
fi

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
fi

popd >/dev/null
