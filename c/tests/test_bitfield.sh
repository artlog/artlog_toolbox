#!/bin/bash


testdir=test_bitfield
mkdir $testdir
prefix=$testdir/bitfield_

for (( i=0; i < 32; i++ ))
do
    echo "=== $prefix$i ==="
    ../../build/bitfield outfile=$prefix$i shuffle=$i debug >$prefix$i.out 2>&1
    echo "*** check ***"
    echo -n "$prefix$i:"
    cmp -l $prefix$i ../../build/bitfield | wc -l
done
