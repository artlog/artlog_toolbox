#!/bin/bash

rootdir=../../

[[ -d tmp ]] || mkdir tmp

abnf=testrange

if [[ -f ${rootdir}abnf/$abnf.abnf ]]
then
    ${rootdir}build/alabnf infile=${rootdir}abnf/$abnf.abnf 2>tmp/alabnf.$abnf.err
    echo '0123456789' | ${rootdir}build/alabnf_matcher 50 ${rootdir}abnf/$abnf.abnf 2>tmp/alabnf_matcher.$abnf.1.err
    echo 'azerty' | ${rootdir}build/alabnf_matcher 50 ${rootdir}abnf/$abnf.abnf 2>tmp/alabnf_matcher.$abnf.2.err
else
    echo "[ERROR] missing ${rootdir}abnf/$abnf.abnf" >&2
fi 


abnf=testref2

if [[ -f ${rootdir}abnf/$abnf.abnf ]]
then
    ${rootdir}build/alabnf infile=${rootdir}abnf/$abnf.abnf 2>tmp/alabnf.$abnf.err
    echo '0123456789' | ${rootdir}build/alabnf_matcher 50 ${rootdir}abnf/$abnf.abnf 2>tmp/alabnf_matcher.$abnf.1.err
    echo 'word word word' | ${rootdir}build/alabnf_matcher 50 ${rootdir}abnf/$abnf.abnf 2>tmp/alabnf_matcher.$abnf.2.err
    echo 'word word ward' | ${rootdir}build/alabnf_matcher 50 ${rootdir}abnf/$abnf.abnf 2>tmp/alabnf_matcher.$abnf.2.err
else
    echo "[ERROR] missing ${rootdir}abnf/$abnf.abnf" >&2
fi 

if false
then
   
    ${rootdir}build/alabnf infile=${rootdir}abnf/rfc3986_part.abnf 2>tmp/alabnf_matcher.1.err

    #${rootdir}build/alabnf_matcher 5000 ${rootdir}abnf/testrange.abnf

    #${rootdir}build/alabnf_matcher ${rootdir}abnf/rfc3986_part.abnf ${rootdir}abnf/rfc5234_core.abnf

    echo "http" | ${rootdir}build/alabnf_matcher 5000 ${rootdir}abnf/rfc5234_core.abnf 2>tmp/alabnf_matcher.2.err

fi
