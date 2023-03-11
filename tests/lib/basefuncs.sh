#!/bin/bash

log_any()
{
    echo "$@" >&2
}

log_error()
{
    log_any "[ERROR] $@"
    errors="$errors $@"
}

log_warn()
{
    log_any "[WARNING] $@"
}

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
