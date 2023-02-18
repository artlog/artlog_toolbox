#!/bin/bash


hashtree=../../build/hashtree

if [[ ! -f $hashtree ]]
then
    echo "[ERROR] missing $hashtree" >&2
    echo "[INFO] hint : cd ../../c; make" >&2
    exit 1
fi

rm out.dot
rm root0x*.dot

$hashtree 'ceci est la premiere phrase' 'la seconde phrase' 'la troisieme phrase' '4' '5' '6' 'sept' '8' '9' '10' '11' '12' '13' '14' '15' '16' '17' 'et la fin ?'

gvpr -f split.gvpr out.dot

dotty *_18.dot
