#!/bin/bash

function update_migration()
{
    if [[ ! -e deploy.version ]]
    then
	# .git/refs/heads/master >deploy.version
	echo "54afe8d9468359e80322283cc5b6b7cd0cb6a630" >deploy.version
    fi

    deployed_version=$(< deploy.version)

    if [[ "$deployed_version" == "54afe8d9468359e80322283cc5b6b7cd0cb6a630" ]]
    then
	if [[ ! -e README.md ]]
	then
	    echo "[INFO] migration README -> README.md"
	    mv README README.md
	fi
    fi

    cp ${A_TOOLBOX}/.git/refs/heads/master deploy.version
}

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
    GIGN=../.gitignore
    [[ -e $GIGN ]] | touch $GIGN
    if grep "^${A_TOOLBOX}" $GIGN
    then
	echo "${A_TOOLBOX} already ignored"
    else
	echo "${A_TOOLBOX}" >> $GIGN
    fi
    for l in $(ls scripts)
    do
	if grep "^$l" $GIGN
	then
	    echo "$l already ignored"
	else
	    echo "$l" >>$GIGN
	fi
    done
    cp scripts/* ..
    popd

    echo "Update migration"
    update_migration
    
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
