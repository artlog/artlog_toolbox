#!/bin/bash

# test code setup to validate small library to move files aways
#    prefix.extension will be moved to prefix.1.extension
#    and any overlapping prefix.i.extension will be moved to prefix.(i+i).extension.
    
dir=savetest
rm -rf $dir

prefix=save
extension=test

mkdir $dir
if [[ -d $dir ]]
then
    pushd $dir
    touch $prefix.$extension
    for ((i=1;i<45;i++));
    do
	touch $prefix.$i.$extension
    done
    rm $prefix.20.$extension
    rm $prefix.30.$extension
    popd
fi
