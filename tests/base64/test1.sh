#!/bin/bash

# please note :
#
# base64_dbg should be build
# cd ../../c; make ../build/base64_dbg

source ../lib/basefuncs.sh

check_diff_files()
{
    fref="$1"
    ftest="$2"
    if ! diff $fref $ftest
    then
	log_error unexpected diff '(<) is ref' "'$fref'" 'and (>) is testing' "'$ftest'"
	log_error hint meld $fref $ftest
	if [[ -n $automeld ]]
	then
	    $automeld $fref $ftest&
	fi
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

encode_decode_test()
{
    reftest=$1
    $base64 -e in=ref/$reftest out=${TESTOUT}/$reftest.b64 2>${TESTOUT}/$reftest.b64.stderr
    check_diff $reftest.b64
    $base64 -e in=ref/$reftest out=${TESTOUT}/$reftest.2.b64 2>${TESTOUT}/$reftest.2.b64.stderr

    if false
    then
	# difference can be seen due to out of bound views might not be relevant
	check_diff_files ${TESTOUT}/$reftest.b64.stderr ${TESTOUT}/$reftest.2.b64.stderr
    fi

    $base64 -d in=${TESTOUT}/$reftest.b64 out=${TESTOUT}/$reftest 2>${TESTOUT}/$reftest.stderr
    check_diff $reftest
}

full_test()
{

    clean_test

if [[ ! -x $base64 ]]
then
    log_error "'$base64' is not executable'"
    exit $EXITERROR
fi

check_executable "$base64"

mkdir ${TESTOUT}

$base64 -h >${TESTOUT}/usage.txt 2>${TESTOUT}/usage.txt.stderr

check_diff usage.txt
check_diff usage.txt.stderr

# don't even try to run executable if usage differs.
check_errors


echo -n 'a' > ${TESTOUT}/charfile.txt

if true
then
    for loop in 1 2
    do
	$base64 -e in=${TESTOUT}/charfile.txt out=${TESTOUT}/charfile.b64 2>${TESTOUT}/charfile.b64.stderr.$loop
    done
    for loop in 1 2
    do
	$base64 -d in=${TESTOUT}/charfile.b64 out=${TESTOUT}/charfile.txt.$loop 2>${TESTOUT}/charfile.txt.stderr.$loop
    done

    # difference can be seen due to out of bound views might not be relevant
    if false
    then
       check_diff_files ${TESTOUT}/charfile.b64.stderr.1 ${TESTOUT}/charfile.b64.stderr.2
       if [[ -n $errors ]]
       then
	   $automeld  ${TESTOUT}/charfile.b64.stderr.1 ${TESTOUT}/charfile.b64.stderr.2
       fi
    fi
       
    check_diff_files ${TESTOUT}/charfile.txt.stderr.1 ${TESTOUT}/charfile.txt.stderr.2

    check_diff_files ${TESTOUT}/charfile.txt ${TESTOUT}/charfile.txt.1

fi

$base64 -e in=ref/plaintext.txt out=${TESTOUT}/plaintext.b64 2>${TESTOUT}/plaintext.b64.stderr
$base64 -e in=ref/plaintext.txt out=${TESTOUT}/plaintext.b64.2 2>${TESTOUT}/plaintext.b64.2.stderr

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

encode_decode_test gnubase64.txt

}

BUILD=../../build
EXITERROR=1

#automeld=meld
automeld=

#base64=$BUILD/base64
#base64=$BUILD/base64_dbg

for fragrance in base64_dbg base64
do    
    echo -e "\n\n******** TESTING $fragrance **********\n\n"
    TESTOUT=tmp.$fragrance
    base64=$BUILD/$fragrance
    full_test
done

check_errors
