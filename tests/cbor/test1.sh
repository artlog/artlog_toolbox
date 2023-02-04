#!/bin/bash

# cbor test copied from base64

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
    srcext=$2
    dstext=$3
    if [[ $srcext == "json" ]]
    then
	inform="inform=$srcext"
    else
	inform=
    fi
    $cbor $inform infile=ref/$reftest.$srcext outfile=${TESTOUT}/$reftest.$dstext 2>${TESTOUT}/$reftest.$dstext.stderr
    check_diff $reftest.$dstext
    $cbor $inform infile=ref/$reftest.$srcext outfile=${TESTOUT}/$reftest.2.$dstext 2>${TESTOUT}/$reftest.2.$dstext.stderr

    if false
    then
	# difference can be seen due to out of bound views might not be relevant
	check_diff_files ${TESTOUT}/$reftest.$dstext.stderr ${TESTOUT}/$reftest.2.$dstext.stderr
    fi

    $cbor infile=${TESTOUT}/$reftest.$dstext outfile=${TESTOUT}/$reftest.$srcext 2>${TESTOUT}/$reftest.$srcext.stderr
    check_diff $reftest.$srcext
}

full_test()
{

    clean_test

if [[ ! -x $cbor ]]
then
    log_error "'$cbor' is not executable'"
    exit $EXITERROR
fi

mime_file=$(file --mime $cbor)

mime_ref="$cbor: application/x-pie-executable; charset=binary"

if [[ $mime_file != $mime_ref ]]
then
    log_error "'$mime_file' != '$mime_ref'"
fi

mkdir ${TESTOUT}

$cbor >${TESTOUT}/usage.txt 2>${TESTOUT}/usage.txt.stderr 

check_diff usage.txt
check_diff usage.txt.stderr

# don't even try to run executable if usage differs.
check_errors

encode_decode_test one cbor json
encode_decode_test map cbor json

}

BUILD=../../build
EXITERROR=1

#automeld=meld
automeld=

#base64=$BUILD/base64
#base64=$BUILD/base64_dbg

for fragrance in cbor_main
do    
    echo -e "\n\n******** TESTING $fragrance **********\n\n"
    TESTOUT=tmp.$fragrance
    cbor=$BUILD/$fragrance
    full_test
done

check_errors
