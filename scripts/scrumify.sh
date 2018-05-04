#!/bin/bash
# Create a scrum hierarchy for project management

usage()
{
    echo "$0 usage:"
    head -n 2 $0
}

usage

SCRUMDIR=scrum

if [[ -d ${SCRUMDIR} ]]
then
    echo "[INFO] ${SCRUMDIR} directory already exists, skipping" >&2
    exit 0
fi

mkdir ${SCRUMDIR}
pushd ${SCRUMDIR} >/dev/null
mkdir -p requirements/user_stories
mkdir roles
mkdir -p backlog/sprint
mkdir -p backlog/product
mkdir deliverables
mkdir validation
popd ${SCRUMDIR} >/dev/null
