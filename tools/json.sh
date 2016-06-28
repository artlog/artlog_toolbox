#!/bin/bash

push_end()
{
    local next=${#expect_end[@]}
    expect_end[$next]=$1
    compute_indent
    parent=
    indent_level[$next]=$indent
    index_level[$next]=$index
    index=0
}

# not used, tentative to display structure naming
compute_indent1()
{
    local end=$(( ${#expect_end[@]} - 2))
    indent=
    if (( $end > -1 ))
    then
	indent="${indent_level[$end]}"
	if [[ -n $parent ]]
	then
	    indent+="/$parent"
	fi
	if [[ ${expect_end[$end]} == ']' ]]
	then
	    indent+="[$index]"
	fi
    fi
}

compute_indent()
{
    local end=$(( ${#expect_end[@]} - 2))
    local space=""
    indent=
    if (( $end > -1 ))
    then
	indent="${indent_level[$end]}"
	for (( i=0 ; i<${#parent} ; i++ ))
	    {
		space+=" "
	    }
	indent+=$space
    fi
}

pop_end()
{
    local end=$(( ${#expect_end[@]} - 1))
    unset expect_end[$end]
    indent=${indent_level[$end]}
    unset indent_level[$end]
    index=${index_level[$end]}
    unset index_level[$end]
    parent=
}

# not activated, for debug purposes
pjson()
{
    echo "${1}" >&3
    indentnl
    return
}

# not activated, for debug purposes
indent()
{
    echo >&3
    indentnl
}

# not activated, for debug purposes
indentnl()
{
    echo -n "$indent" >&3
}

indentnl1()
{
    local last=$(( ${#expect_end[@]} - 1 ))

    if (( $last < 0 ))
    then
	echo -n "$indent" >&3
	return
    fi

    if [[ ${expect_end[$last]} == ']' ]]
    then
	echo -n "$indent[$index]" >&3
    else
	echo -n "$indent" >&3
    fi
}

pjsonword()
{
    # not really good escape for json, escape more characters ( like ' ' '(' ')' ... )
    printf "\"%q\"" "$1" >&3
}

# not activated, for debug purposes
pjsonnl()
{
    echo -n $1 >&3
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

# slow manual way, prefer readword relying on bash read escaping feature.
readword1()
{
    word=
    readchar
    while [[ ! "$C" == "$1" ]]
    do
	word="$word$C"
	readchar
	# should handle escaped char...
	if [[ $C == "\\" ]]
	then
	    escapechar
	    word=$word$C
	    C=
	fi
    done
    pjsonnl "$word" $1
}

readword()
{
    # use bash read escaping features...
    IFS= read -d "$1" word
}

escapechar()
{
    
#    \b  Backspace (ascii code 08)
#    \f  Form feed (ascii code 0C)
#    \n  New line
#    \r  Carriage return
#    \t  Tab
#    \"  Double quote
    #    \\  Backslash caracter
    readchar
    #TODO
    return
}
   

json_capture_arg()
{

    capture_arg=$1

    EXCEED=
    capture=0
    index=0

    readchar
    
    while [[ $do_it != 0 ]]
    do
	if [[ "$C" =~ [\"\'] ]]
	then
	    readword "$C"
	    if [[ $capture == 1 ]]
	    then
		echo "$word"
		capture=0
	    fi
	    pjsonword "$word" $C
	elif [[ "$C" =~ [0-9] ]]
	then
	    word=$C
	    readchar
	    while [[ "$C" =~ [0-9] ]]
	    do
		word="$word$C"
		readchar
	    done
	    pjsonnl "$word" int
	    EXCEED=$C
	elif [[ "$C" =~ [a-zA-Z] ]]
	then
	    word=$C
	    readchar
	    while [[ "$C" =~ [a-zA-Z] ]]
	    do
		word="$word$C"
		readchar
	    done
	    pjsonnl "$word" token
	    EXCEED=$C
	elif [[ "$C" == ':' ]]
	then
	    capture=0
	    if [[ $word == $capture_arg ]]
	    then
		capture=1
	    fi
	    pjsonnl "$C"
	    parent=$word
	elif [[ "$C" == ',' ]]
	then
	    capture=0
	    index=$(( index + 1 ))
	    pjson "$C"
	elif [[ "$C" =~ '{' ]]
	then
	    # start struct, should exit it only if find '}'
	    push_end '}'
	    pjson $C
	elif [[ "$C" == '[' ]]
	then
	    # start array, should exit it only if find ']'
	    push_end ']'
	    pjson $C
	elif [[ $C == ${expect_end[$(( do_it - 1 ))]} ]]
	then
	    pop_end
	    indent
	    pjsonnl "$C"
	elif [[ "$C" == "]" ]]
	then
	    echo "ERROR unexpected ']' here" >&2
	elif [[ "$C" == "}" ]]
	then
	    echo "ERROR unexpected '}' here" >&2
	else
	    pjsonnl "$C"
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

if [[ -n $debugprint ]]
then
    if [[ ! -f $debugprint ]]
    then
	touch $debugprint
    fi	
    exec 3>$debugprint
else
    exec 3>/dev/null
fi

json_capture_arg name
