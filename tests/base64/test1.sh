#!/bin/bash

log_error()
{
    echo "[ERROR] $@" >&2
    errors="$errors $@"
}

clean_test()
{
    rm -r ./${TESTOUT}
}

check_diff()
{
    filetotest=$1
    if ! diff ref/$filetotest ${TESTOUT}/$filetotest 
    then
	log_error unexpected $filetotest diff '(<) is ref and (>) is testing output'
    fi
}

check_errors()
{
    [[ -n $errors ]] && echo "$errors" &&  exit $EXITERROR
}

BUILD=../../build
TESTOUT=tmp
EXITERROR=1

clean_test

base64=$BUILD/base64

if [[ ! -x $base64 ]]
then
    log_error "'$base64' is not executable'"
    exit $EXITERROR
fi

mime_file=$(file --mime $base64)

mime_ref="$base64: application/x-pie-executable; charset=binary"

if [[ $mime_file != $mime_ref ]]
then
    log_error "'$mime_file' != '$mime_ref'"
fi

mkdir ${TESTOUT}

$base64 >${TESTOUT}/usage.txt 2>${TESTOUT}/usage.txt.stderr 

check_diff usage.txt
check_diff usage.txt.stderr

# don't even try to run executable if usage differs.
check_errors

$base64 -e in=ref/plaintext.txt out=${TESTOUT}/plaintext.b64

check_diff plaintext.b64

$base64 -d in=ref/plaintext.b64 out=${TESTOUT}/plaintext.txt
    
check_diff plaintext.txt

if ! diff  ${TESTOUT}/plaintext.b64 ref/plaintext.b64 >/dev/null
then
    echo "Due to ref error checking reverse on generated output"
    mv ${TESTOUT}/plaintext.txt ${TESTOUT}/plaintext.txt.1

    $base64 in=${TESTOUT}/plaintext.b64 out=${TESTOUT}/plaintext.txt
    
    check_diff plaintext.txt

    diff ${TESTOUT}/plaintext.txt ${TESTOUT}/plaintext.txt.1
fi


check_errors
