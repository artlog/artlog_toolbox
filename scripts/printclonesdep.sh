#!/bin/bash
# output list of clones and current git commit references
# for this project and projects dependencies ( clones.dep )

get_git_info()
{
    local clone=$1
    echo "[$clone]"
    echo "path=$(realpath --relative-to=$absolute $(pwd))"
    echo "remote.origin.url=$(git config --get remote.origin.url)"
    echo "head=$(< .git/$(git symbolic-ref HEAD))"
    # echo "branch=$(git branch)"
}

absolute=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

pushd $absolute >/dev/null

echo "#"
echo "# root.absolute=$absolute"
echo "# date=$(date)"

project_name=.

if [[ -f project_params ]]
then
    project_name=$(grep "project_name" project_params)
    project_name=${project_name/project_name=/}
fi

# expect . to be in cube
get_git_info $project_name

if [[ -f  clones.dep ]]
then
    clones=$(< clones.dep)
    for clone in $clones
    do
	directory=../$clone
	if [[ -d $directory ]]
	then
	    pushd $directory >/dev/null
	    get_git_info $clone
	    popd >/dev/null
	else
	    echo "[ERROR] clone $clone not found in $directory" >&2
	fi
    done
else
    echo "[INFO] no clone.deps file" >&2
fi

popd >/dev/null
