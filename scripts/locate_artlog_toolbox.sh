#!/bin/bash

select_artlog_toolbox()
{
    directory_name=${1:-artlog_toolbox}
    potential_path=($(find .. -type d -name "$directory_name"))

    # default to this.
    ARTLOG_TOOLBOX=$(pwd)

    toolboxparam=./toolbox.param
    if [[ -f $toolboxparam ]]
    then
	echo "source $toolboxparam"
	source $toolboxparam
    fi
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
    echo "[WARNING] function tool used as main" >&2
    select_artlog_toolbox $1
    exit 0
fi
