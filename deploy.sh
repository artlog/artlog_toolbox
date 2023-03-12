#!/bin/bash

# .git/refs/heads/master >deploy.version
git_commit_ref='54afe8d9468359e80322283cc5b6b7cd0cb6a630'

usage()
{
    echo "$0 tool is intended to be use to copy all toolbox scripts within current directory with some reference to toolbox version"

    echo "destroy         removve all scripts deployed by this tool"
    echo "h|-h|--h|help   this help"
    echo "copy            default is to deploy scripts"
}

# search ARTLOG_TOOLBOX= within $toolboxparam file, no bash expansion is done.
# copied from scripts/locate_artlog_toolbox.sh but required here since bootstrap
extract_from_toolbox_param()
{
    local toolboxparam=$1
    if [[ -f $toolboxparam ]]
    then
	echo "extract ARTLOG_TOOLBOX from $toolboxparam" >&2
	while read LINE
	do
	    if [[ $LINE =~ ARTLOG_TOOLBOX=(.*) ]]
	    then
		ARTLOG_TOOLBOX=${BASH_REMATCH[1]}
	    fi
	done <$toolboxparam
    fi
}

function update_migration()
{
    if [[ ! -e deploy.version ]]
    then
	echo "$git_commit_ref" >deploy.version
    fi

    deployed_version=$(< deploy.version)

    if [[ "$deployed_version" == "$git_commit_ref" ]]
    then
	if [[ ! -e README.md ]]
	then
            if [[ -e README ]]
            then
	        echo "[INFO] migration README -> README.md"
	        mv README README.md
            fi
	fi
    fi

    # really update deploy.version
    cp ${A_TOOLBOX}/.git/refs/heads/master deploy.version
}

destroy=0

if [[  $#  == 0 ]]
then
    echo "[INFO] use -help to get help"
    exit 0
fi

while [[ $# -gt 0 ]]
do
    case $1 in
	destroy)
	    destroy=1
	    ;;
	h|-h|--h|help|-help|--help)
	    usage
	    exit 1
	    ;;
	copy)
	    copy=1
	    ;;
	*)
	    echo "[ERROR] unrecognized '$1' argument for $0" >&2	    
	    exit 1
	;;
    esac
    shift 1
done


PROJECT_DIR=$(pwd)

# ARTLOG_TOOLBOX can be exported as environment variable
if [[ -z $ARTLOG_TOOLBOX ]]
then
    # or can be set within local project toolbox.param
    if [[ -e toolbox.param ]]
    then
        echo "[WARNING] This is not the first deploy since toolbox.param file exists here" >&2
        extract_from_toolbox_param toolbox.param
    fi
fi

if [[ -z $ARTLOG_TOOLBOX ]]
then
    A_TOOLBOX=$(dirname $(readlink -f "$0"))
    echo "find toolbox relative to this deploy.sh script '$0'"
else
    A_TOOLBOX=$ARTLOG_TOOLBOX
fi
echo "ARTLOG_TOOLBOX=$A_TOOLBOX"

if [[ ! -d $A_TOOLBOX ]]
then
    echo "[ERROR] this tool should be run from parent dir of $A_TOOLBOX this way : ${A_TOOLBOX}/deploy.sh" >&2
    exit 1
fi

if [[ $destroy == 1 ]]
then
    echo "destroy deployed toolbox"
    for script in $(ls ${A_TOOLBOX}/scripts)
    do
	if [[ -f $script ]]
	then
	    shortscript=$(basename "$script")
            echo "deleting '$shortscript'"
	    rm "${PROJECT_DIR}/$shortscript"
	fi
    done
fi

if [[ $copy == 1 ]]
then
    echo "[INFO] Deploying scripts with copy..." >&2

    pushd  ${A_TOOLBOX}
    GIGN=${PROJECT_DIR}/.gitignore
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
    # copy only shell scripts and makefile
    cp scripts/*.sh scripts/*.makefile ${PROJECT_DIR}/
    popd

    echo "Update migration"
    update_migration
fi
