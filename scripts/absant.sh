#!/bin/bash

usage()
{
    echo 'once upon a time i needed to replace ${ opening brace to \${ from a build.xml to stdout'
    echo "I don't remember why but given the name of this script it was for ant build"
    echo "still first argument given is used only to switch from error to actual process, not used as variable, this look like this script writing was not fully completed. Maybe used to generate template embedded in antify.sh"
}

buildscript=$1

if [[ -e $buildscript ]]
then
    # protect ${ variables.
    sed 's/\${/\\\${/g' build.xml
else
    echo "[ERROR] this $0 tool expect an initial argument with buildscript" >&2
    usage
fi
    
