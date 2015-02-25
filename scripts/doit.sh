#!/bin/bash

setup()
{
    if [[ ! -d java ]]
    then
	echo "[FATAL] this tools $0 is intended to be run within laby project. Missing java directory." >&2
	exit 1
    fi
    if [[ ! -d lab ]]
    then
	echo "[INFO] Creating directory lab to save default lab created"
	mkdir lab
    fi
}

do_code()
{
    background=$1
    find java/org/artisanlogiciel/games/ -name "*.java" |
	{
	    s=()
	    while read codeline
	    do
		javafile=$(basename "$codeline")
		javaclass=${javafile/.java/}
		echo "$javafile $javaclass"
		s+=("$javaclass" "$codeline")
	    done
	    javaclass=$($DIALOG --menu "Edit it" 20 100 10 ${s[@]} 3>&1 1>&2 2>&3)
	    if [[ -n $javaclass ]]
	    then
		pushd java
		if [[ $background == codebg ]]
		then
		    nohup make work/$javaclass &
		else
		    make work/$javaclass
		fi
		popd
	    fi
	}
    
}

edit_properties()
{
    local property_file=$1
    if [[ -f $property_file ]]
    then
	modified=0
	s=()
	properties=()
	values=()
	{
	    while read codeline
	    do
		if [[ $codeline =~ (^[a-zA-Z_]*)=(.*) ]]
		then
		    property=${BASH_REMATCH[1]}
		    value=${BASH_REMATCH[2]}
		    s+=("$property" "$value")
		    properties+=("$property")
		    values+=("$value")
		fi
	    done
	    s+=(exit "Exit")
	} < $property_file
	
	while true 
	do
	    property=$($DIALOG --menu "Edit '$property_file'" 20 100 10 ${s[@]} 3>&1 1>&2 2>&3)
	    if [[ $? = 0 ]]
	    then
		if [[ -n $property ]]
		then
		    if [[ $property == exit ]]
		    then
			return 1
		    elif [[ $property == save ]]
		    then
			for  (( i=0; i<${prop_len}; i++ ));
			do
			    echo "${properties[$i]}=${values[$i]}"
			done >$property_file
			return 0
		    fi
		    prop_len="${#properties[@]}"
		    for  (( i=0; i<${prop_len}; i++ ));
		    do
			if [[ ${properties[$i]} == $property ]]
			then
			    init_value=${values[$i]}
			fi
		    done
		    value=$($DIALOG --inputbox "Enter $property value" 10 80 "$init_value" 3>&1 1>&2 2>&3)
		    if [[ $? = 0 ]]
		    then
			prop_len="${#properties[@]}"
			s=()
			if [[ "$value" != "$init_value" ]]
			then
			    modified=$(( modified + 1 ))
			fi
			for  (( i=0; i<${prop_len}; i++ ));
			do
			    if [[ ${properties[$i]} == $property ]]
			    then
				values[$i]=$value
			    fi
			    s+=("${properties[$i]}" "${values[$i]}")
			    echo "${properties[$i]}=${values[$i]}"
			done
			s+=(exit "Exit")
			if [[ $modified != 0 ]]
			then
			    s+=(save "Save")
			fi
		    fi
		fi
	    else
		return 2
	    fi
	done
    else
	echo "[ERROR] property_file '$propertyfile' not found" >&2
    fi

}

# first argument mandatory : directory containing generated .lab and .stl files ( usualy ./lab )
list_labs()
{
    local lab_dir=$1
    if [[ -d $lab_dir ]]
    then
	modified=0
	s=()
	properties=()
	values=()
	{
	    pushd $lab_dir
	    for stl_lab in $(ls *lab*.stl)
	    do
		if [[ $stl_lab =~ ^([^0-9]*[0-9]*x[0-9]*).stl ]]
		then
		    property=${BASH_REMATCH[1]}
		    value=$stl_lab
		    s+=("$property" "$value")
		    properties+=("$property")
		    values+=("$value")
		fi
	    done
	    s+=(exit "Exit")
	    popd
	}
	
	while true 
	do
	    property=$($DIALOG --menu "Show stl file from $lab_dir" 20 100 10 ${s[@]} 3>&1 1>&2 2>&3)
	    if [[ $? = 0 ]]
	    then
		if [[ -n $property ]]
		then
		    if [[ $property == exit ]]
		    then
			return 1
		    elif [[ $property == save ]]
		    then
			echo "TODO"
			return 0
		    fi
		    prop_len=${#properties[*]}
		    for  (( i=0; i<${prop_len}; i++ ));
		    do
			if [[ ${properties[$i]} == $property ]]
			then
			    blender --python blender_import.py -- $lab_dir/${values[$i]}
			fi
		    done

		fi
	    else
		return 2
	    fi
	done
    else
	echo "[ERROR] lab dir '$propertyfile' not found" >&2
    fi

}

possible_console_gui="whiptail dialog"

for DIALOG in $possible_console_gui
do
    DIALOG=$(which $DIALOG)
    if [[ -n $DIALOG ]]
    then
	break
    fi
done

if [[ -z $DIALOG ]]
then
    echo "[ERROR] no console gui support (no dialog tool found within $possible_console_gui)  => no menus " >&2
    exit 1
fi

setup

action=initial

while [[ $action != quit ]]
do
    action=$($DIALOG --menu "Ultra Light IDE" 20 80 12 readme "Read me" clean "Clean All" ant "Ant build" run "Run it"  list_labs "Show Labyrinth with blender" test "Test it" code "Code" codebg "Code in background" deb "Debian package" properties "Edit Properties" create "Create a new class" emacsdevenv "Setup Emacs java bindings JDEE" quit "Quit" 3>&1 1>&2 2>&3)

    if [[ $action == run ]]
    then
	echo "run it"
	java -jar $(make getname)
    elif [[ $action == ant ]]
    then
	make clean
	make
	ant compile
    elif [[ $action == clean ]]
    then
	make clean
        pushd java
	make clean
	popd
    elif [[ $action == test ]]
    then
	echo "test it"
	pushd java
	make display
	popd
    elif [[ $action == deb ]]
    then
	make deb
    elif [[ $action =~ code ]]
    then
	do_code $action
    elif [[ $action == readme ]]
    then
	$DIALOG --textbox README 40 80 --scrolltext
    elif [[ $action == properties ]]
    then
	edit_properties project_params
    elif [[ $action == create ]]
    then
	newclass=$($DIALOG --inputbox "Enter new class name" 10 80 "NewClass" 3>&1 1>&2 2>&3)
	if [[ $? = 0 ]]
	then
	    if [[ -n $newclass ]]
	    then
		pushd java
		make work/$newclass
		popd
	    fi
	fi
    elif [[ $action == list_labs ]]
    then
	list_labs ./lab
    elif [[ $action == emacsdevenv ]]
    then
	make $action
    fi
done
