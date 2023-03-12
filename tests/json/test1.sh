#!/bin/bash

# json test copied from cbor

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
    srcext=$2
    dstext=$3
    if [[ $srcext == "json" ]]
    then
	inform="inform=$srcext"
    else
	inform=
    fi
    # $json $inform infile=ref/$reftest.$srcext out=${TESTOUT}/$reftest.$dstext 2>${TESTOUT}/$reftest.$dstext.stderr
    $json $inform indent=spaces -- ref/$reftest.$srcext >${TESTOUT}/$reftest.$dstext 2>${TESTOUT}/$reftest.$dstext.stderr
    check_diff $reftest.$dstext
    # $json $inform infile=ref/$reftest.$srcext out=${TESTOUT}/$reftest.2.$dstext 2>${TESTOUT}/$reftest.2.$dstext.stderr
    $json $inform indent=spaces -- ref/$reftest.$srcext >${TESTOUT}/$reftest.2.$dstext 2>${TESTOUT}/$reftest.2.$dstext.stderr

    if true
    then
	# difference can be seen due to out of bound views might not be relevant
	check_diff_files ${TESTOUT}/$reftest.$dstext.stderr ${TESTOUT}/$reftest.2.$dstext.stderr
    fi

    # $json infile=${TESTOUT}/$reftest.$dstext out=${TESTOUT}/$reftest.$srcext 2>${TESTOUT}/$reftest.$srcext.stderr
    $json indent=none -- ${TESTOUT}/$reftest.$dstext >${TESTOUT}/$reftest.$srcext 2>${TESTOUT}/$reftest.$srcext.stderr
    check_diff $reftest.$srcext

    log_any "[INFO] encode decode json $reftest $srcext $dstext '$errors'"
}

runit()
{
    echo "$@"
    $@
}

full_test()
{

    clean_test

    if [[ ! -x $json ]]
    then
	log_error "'$json' is not executable'"
	exit $EXITERROR
    fi

    check_executable "$json"

    mkdir ${TESTOUT}

    $json >${TESTOUT}/usage.txt 2>${TESTOUT}/usage.txt.stderr 

    check_diff usage.txt
    check_diff usage.txt.stderr

    # don't even try to run executable if usage differs.
    check_errors

    $json indent=spaces -- ref/Q3390720.json.pp >${TESTOUT}/Q3390720.json.pp

    check_diff Q3390720.json.pp 


    #encode_decode_test one cbor json
    #encode_decode_test map cbor json

    # $json -d -- ref/appendix_a.json

    # fully recursive
    runit $json maxdepth=50 -- ref/syntax_error_101.json

    # fully non recursive
    runit $json maxdepth=20000 -- ref/syntax_error_101.json

    # this shows that mix recursive & non_recursive has hacks
    # $json -d -- ref/syntax_error_101.json

    # mix recursive and non recursive
    runit $json -- ref/syntax_error_101.json

    encode_decode_test backslash json pp.json

    
}

BUILD=../../build
EXITERROR=1

#automeld=meld
automeld=

#base64=$BUILD/base64
#base64=$BUILD/base64_dbg

for fragrance in json
do    
    echo -e "\n\n******** TESTING $fragrance **********\n\n"
    TESTOUT=tmp.$fragrance
    json=$BUILD/$fragrance
    full_test
done

check_errors
