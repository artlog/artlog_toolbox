#!/bin/bash
# Under LGPL License

if [[ -f toolbox.param ]]
then    
    while read LINE
    do
	if [[ $LINE =~ ARTLOG_TOOLBOX=(.*) ]]
	then
	    ARTLOG_TOOLBOX=${BASH_REMATCH[1]}
	fi
    done <toolbox.param
fi

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

