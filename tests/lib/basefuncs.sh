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
    
    mime_file=$(file --mime "$bintocheck")

    mime_ref="$bintocheck: application/x-pie-executable; charset=binary"
    mime_ref2="$bintocheck: application/x-sharedlib; charset=binary"

    if [[ $mime_file != $mime_ref ]]
    then
	log_warn  "'$mime_file' != '$mime_ref'"
	if [[  $mime_file != $mime_ref2 ]]
	then
	    log_error "'$mime_file' != '$mime_ref2'"
	fi
    fi

}
