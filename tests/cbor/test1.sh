#!/bin/bash

# cbor test copied from base64

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
    echo "[TEST] $reftest $srcext -> $dstext"
    inform="inform=$srcext"
    outform="inform=$dstext"
  
    run $cbor $inform infile=ref/$reftest.$srcext outfile=${TESTOUT}/$reftest.$dstext 2>${TESTOUT}/$reftest.$dstext.stderr
    check_diff $reftest.$dstext
        
    if false
    then
	run $cbor $inform infile=ref/$reftest.$srcext outfile=${TESTOUT}/$reftest.2.$dstext 2>${TESTOUT}/$reftest.2.$dstext.stderr

	# difference can be seen due to out of bound views might not be relevant
	check_diff_files ${TESTOUT}/$reftest.$dstext.stderr ${TESTOUT}/$reftest.2.$dstext.stderr
    fi

    run $cbor $outform infile=${TESTOUT}/$reftest.$dstext outfile=${TESTOUT}/$reftest.$srcext 2>${TESTOUT}/$reftest.$srcext.stderr
    check_diff $reftest.$srcext
}

full_test()
{

    clean_test

    if [[ ! -e $cbor ]]
    then
	log_error "$cbor does not exists"
	log_warn "Your might want to do: cd ../../c/cbor; make"
    fi
    
    if [[ ! -x $cbor ]]
    then
	log_error "'$cbor' is not executable'"    
	exit $EXITERROR
    fi

    check_executable "$cbor"

    mkdir ${TESTOUT}

    $cbor >${TESTOUT}/usage.txt 2>${TESTOUT}/usage.txt.stderr 

    check_diff usage.txt
    check_diff usage.txt.stderr

    # don't even try to run executable if usage differs.
    check_errors

    encode_decode_test one json cbor
    encode_decode_test one cbor json   

    encode_decode_test map json cbor
    encode_decode_test map cbor json

    samplename=Q3390720
    run $cbor inform=json infile=../json/ref/$samplename.json outfile=tmp.cbor_main/$samplename.cbor
    run $cbor inform=cbor infile=tmp.cbor_main/$samplename.cbor outfile=tmp.cbor_main/$samplename.json 


}


run()
{
    echo $@
    $@
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
