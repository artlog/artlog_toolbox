#!/bin/bash

echo "Create a scrum hierarchy for project management"

mkdir scrum
pushd scrum >/dev/null
mkdir -p requirements/user_stories
mkdir roles
mkdir -p backlog/sprint
mkdir -p backlog/product
mkdir deliverables
mkdir validation
popd scrum >/dev/null
