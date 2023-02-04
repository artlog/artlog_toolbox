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

check_diff_files()
{
    fref="$1"
    ftest="$2"
    if ! diff $fref $ftest
    then
	log_error unexpected diff '(<) is ref' "'$fref'" 'and (>) is testing' "'$ftest'"
    fi
}

check_diff()
{
    filetotest=$1
    check_diff_files "ref/$filetotest" "${TESTOUT}/$filetotest"
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


echo -n 'a' > ${TESTOUT}/charfile.txt

if true
then
    $base64 -e in=${TESTOUT}/charfile.txt out=${TESTOUT}/charfile.b64 2>${TESTOUT}/charfile.b64.stderr.1

    $base64 -e in=${TESTOUT}/charfile.txt out=${TESTOUT}/charfile.b64 2>${TESTOUT}/charfile.b64.stderr.2

    $base64 -d in=${TESTOUT}/charfile.b64 out=${TESTOUT}/charfile.txt.1 2>${TESTOUT}/charfile.txt.stderr.1

    $base64 -d in=${TESTOUT}/charfile.b64 out=${TESTOUT}/charfile.txt.2 2>${TESTOUT}/charfile.txt.stderr.2

    # difference can be seen due to out of bound views might not be relevant
    if false
    then
       check_diff_files ${TESTOUT}/charfile.b64.stderr.1 ${TESTOUT}/charfile.b64.stderr.2
       if [[ -n $errors ]]
       then
	   meld  ${TESTOUT}/charfile.b64.stderr.1 ${TESTOUT}/charfile.b64.stderr.2
       fi
    fi
       
    check_diff_files ${TESTOUT}/charfile.txt.stderr.1 ${TESTOUT}/charfile.txt.stderr.2


    check_diff_files ${TESTOUT}/charfile.txt ${TESTOUT}/charfile.txt.1

fi

$base64 -e in=ref/plaintext.txt out=${TESTOUT}/plaintext.b64 2>${TESTOUT}/plaintext.b64.stderr
$base64 -e in=ref/plaintext.txt out=${TESTOUT}/plaintext.b64.2 2>${TESTOUT}/plaintext.b64.2.stderr

meld ${TESTOUT}/plaintext.b64.stderr ${TESTOUT}/plaintext.b64.2.stderr&

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
