#!/bin/bash

push_end()
{
    expect_end[${#expect_end[@]}]=$1
}

pop_end()
{
    unset expect_end[$(( ${#expect_end[@]} - 1))]
}

# not activated, for debug purposes
pjson()
{
#    echo $1
    return
}

# not activated, for debug purposes
pjsonnl()
{
#    echo -n $1
    return
}

readchar()
{
    if ! read -N 1 -r C
    then
	echo "[ERROR] json file too short or malformed" >&2
	exit 1
    fi
}

json_capture_arg()
{

    capture_arg=$1

    EXCEED=
    capture=0

    readchar
    
    while [[ $do_it != 0 ]]
    do
	if [[ "$C" == \" ]]
	then
	    word=
	    readchar
	    while [[ ! "$C" == \" ]]
	    do
		word="$word$C"
		readchar
	    done
	    pjsonnl "'$word'"
	    if [[ $capture == 1 ]]
	    then
		echo "$word"
		capture=0
	    fi
	elif [[ "$C" =~ [0-9] ]]
	then
	    word=$C
	    readchar
	    while [[ "$C" =~ [0-9] ]]
	    do
		word="$word$C"
		readchar
	    done
	    pjsonnl "int($word)"
	    EXCEED=$C
	elif [[ "$C" == ':' ]]
	then
	    capture=0
	    if [[ $word == $capture_arg ]]
	    then
		capture=1
	    fi
	elif [[ "$C" == '{' ]]
	then
	    # start struct, should exit it only if find '}'
	    push_end '}'
	    pjsonnl $C
	elif [[ "$C" == '[' ]]
	then
	    # start array, should exit it only if find ']'
	    push_end ']'
	    pjsonnl $C
	elif [[ $C == ${expect_end[$(( do_it - 1 ))]} ]]
	then
	    pjsonnl "$C #END $do_it"
	    pop_end
	elif [[ "$C" == "]" ]]
	then
	    echo "ERROR unexpected ']' here"
	elif [[ "$C" == "}" ]]
	then
	    echo "ERROR unexpected '}' here"
	else
	    pjson "$C"
	fi

	if [[ -z $EXCEED ]]
	then
	    readchar
	else
	    C=$EXCEED
	    EXCEED=
	fi

	do_it=${#expect_end[@]}
    done
}

declare -a expect_end

json_capture_arg name
