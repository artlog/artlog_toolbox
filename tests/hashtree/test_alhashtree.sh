#!/bin/bash

show_dots() {
    if [[ -f out.dot ]]
    then
	gvpr -f split.gvpr out.dot
	dotty *_0.dot&
    fi
}
    

hashtree=../../build/hashtree

if [[ ! -f $hashtree ]]
then
    echo "[ERROR] missing $hashtree" >&2
    echo "[INFO] hint : cd ../../c; make" >&2
    exit 1
fi

# clean should be done with make clean

$hashtree 'ceci est la premiere phrase' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6' 'sept' '8' '9' '10' '11' '12' '13' '14' '15' '16' '17' 'et la fin ?'

show_dots

../../build/json indent=spaces:2 -- out.json
