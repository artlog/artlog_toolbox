#!/bin/bash
# Under LGPL License

if [[ -f  ./locate_artlog_toolbox.sh ]]
then
    source ./locate_artlog_toolbox.sh
else
    echo "[ERROR] this script rely on ./locate_artlog_toolbox.sh, not found in $(pwd)" >&2
    exit 1
fi

extract_from_toolbox_param toolbox.param

if [[ -n $ARTLOG_TOOLBOX ]]
then
    echo ARTLOG_TOOLBOX=$ARTLOG_TOOLBOX
    if [[ -x $ARTLOG_TOOLBOX/deploy.sh ]]
    then
	# this current script can be altered by deploy, so exec
	exec $ARTLOG_TOOLBOX/deploy.sh
	echo "[ERROR] ($0) should not reach this point" >&2
    else
	echo "[ERROR] $ARTLOG_TOOLBOX/deploy.sh not found/executable" >&2
	echo "[HELP] Please check file toolbox.param ARTLOG_TOOLBOX= to point to artlog toolbox"
    fi
else
    echo "[ERROR] Can't refresh. file toolbox.param not found." >&2
    echo "[HELP] Please create file toolbox.param  with ARTLOG_TOOLBOX= to point to artlog toolbox"
fi

