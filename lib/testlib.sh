#!/bin/bash

clean_test()
{
    rm -r ./${TESTOUT}
}

check_executable()
{
    bintocheck="$1"
    
    declare -a mime_ref
    mime_refs=("$bintocheck: application/x-pie-executable; charset=binary" \
"$bintocheck: application/x-sharedlib; charset=binary" \
"$bintocheck: application/x-executable; charset=binary")

    mime_file=$(file --mime "$bintocheck")
    
    match=0
    arraylength=${#mime_refs[@]}
    for (( i=0; i<${arraylength}; i++ ))
    do
	mime_ref="${mime_refs[$i]}"
	if [[ $mime_file == $mime_ref ]]
	then
	    match=1
	    break;
	else
	    log_warn "'$mime_file' != '$mime_ref'"
	fi
    done
    if (( match == 0 ))
    then
	log_error  "'$mime_file' does not match any expected match"
    fi

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
