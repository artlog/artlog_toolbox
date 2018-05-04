#!/bin/bash

# search ARTLOG_TOOLBOX= within $toolboxparam file, no bash expansion is done.
extract_from_toolbox_param()
{
    local toolboxparam=$1
    if [[ -f $toolboxparam ]]
    then
	echo "extract from $toolboxparam" >&2
	while read LINE
	do
	    if [[ $LINE =~ ARTLOG_TOOLBOX=(.*) ]]
	    then
		ARTLOG_TOOLBOX=${BASH_REMATCH[1]}
	    fi
	done <$toolboxparam
    fi
}

select_artlog_toolbox()
{
    directory_name=${1:-artlog_toolbox}
    potential_path=($(find .. -type d -name "$directory_name"))

    # default to this.
    ARTLOG_TOOLBOX=$(pwd)

    toolboxparam=./toolbox.param

    extract_from_toolbox_param "$toolboxparam"

    echo "Current toolbox : $ARTLOG_TOOLBOX"

    potential_number=${#potential_path[@]}
    if (( $potential_number > 0 ))
    then
	for (( i=0; i<$potential_number; i++ ))
	do
	    echo "$(( i + 1)) ${potential_path[$i]}"
	done
	echo "which one would you like to use (select number 1 to $potential_number) all other for current ?"
	read a
	if [[ -z $a ]]
	then
	    a=0
	fi
	if (( $a > 0 ))
	then
	    ARTLOG_TOOLBOX=$(readlink -f ${potential_path[$(( a - 1))]})
	fi
	echo "$ARTLOG_TOOLBOX"
    else
	echo "[INFO] No directory $directory_name name found"
    fi
}
	    

if [[ $0 =~ locate_artlog_toolbox.sh ]]
then
    echo "[WARNING] function tool $0 used as main" >&2
    if [[ $#  > 0 ]]
    then
	echo "$0 called with with param $1" >&2
	select_artlog_toolbox $1
    else
	extract_from_toolbox_param ./toolbox.param
	if [[ -n $ARTLOG_TOOLBOX ]]
	then
	    echo "$ARTLOG_TOOLBOX"
	else
	    echo "no ARTLOG_TOOLBOX set" >&2
	fi
    fi
    exit 0
fi
