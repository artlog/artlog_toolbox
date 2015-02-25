#!/bin/bash

A_TOOLBOX=artlog_toolbox

if [[ ! -d $A_TOOLBOX ]]
then
    echo "[ERROR] this tool should be run from parent dir of $A_TOOLBOX this way : ${A_TOOLBOX}/deploy.sh" >&2
    exit 1
fi

destroy=0

while [[ $# -gt 0 ]]
do
    case $1 in
	destroy)
	    destroy=1
	    ;;
	*)
	    echo "[ERROR] unrecognized '$1' argument for $0" >&2
	    exit 1
	;;
    esac
    shift 1
done

if [[ $destroy != 1 ]]
then
    echo "Deploying scripts..."

    pushd  ${A_TOOLBOX}
    cp scripts/* ..
    popd
else
    echo "destroy deployed toolbox"
    for script in $(ls ${A_TOOLBOX}/scripts)
    do
	if [[ -f $script ]]
	then
	    shortscript=$(basename "$script")
            echo "deleting '$shortscript'"
	    rm "$shortscript"
	fi
    done
fi
