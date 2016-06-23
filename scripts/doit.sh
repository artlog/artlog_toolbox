#!/bin/bash
# entry point for project building / editing / running

devenv_setup()
{
    if [[ -e devenv_params ]]
    then
	if [[ -e .within-devenv ]]	   
	then
	    echo "[ERROR] pid $(< .within-devenv) already called this $0 script within dev_env_setup" >&2
	    echo "[INFO] Stopping to protect against infinite recursion" >&2
	    echo "[INFO] stop $(< .within-devenv) process and remove .within-devenv file if it remains"
	    exit 1
	else
	    echo "[WARNING] Run with devenv_params"
	    source devenv_params
	    if [[ -z $NOJAVA ]]
	    then
		PATH=$JDK_PATH/bin/:$JDK_PATH/jre/bin/:$ECLIPSE_PATH:$PATH
		JAVA_MAKEFILE=Makefile.4.java
		export PATH JDK_PATH ECLIPSE_PATH DEV_ENV JAVA_MAKEFILE
	    else
		export NOJAVA
	    fi
	    echo "$PID" > .within-devenv
	    # Don't forget to set a param unless it will go in endless recursive call
	    $0 within-devenv
	    EXITCODE=$?
	    rm .within-devenv
	    exit $EXITCODE
	fi
    fi
}

info()
{
    if [[ -z $NOJAVA ]]
    then
	echo "DEV_ENV=$DEV_ENV"
	echo "JDK_PATH=$JDK_PATH"
	echo "ECLIPSE_PATH=$ECLIPSE_PATH"
	echo "PATH=$PATH"
	java -version
    else
	echo "NOJAVA is set to $NOJAVA"
    fi
}

setup()
{
    if [[ -z $NOJAVA ]]
    then
	if [[ ! -d java ]]
	then
	    echo "[INFO] Missing java directory." >&2
	fi
	# FIXME specific to laby project, should not be within general toolbox.
	if [[ ! -d lab ]]
	then
	    echo "[INFO] Creating directory lab to save default lab created"
	    mkdir lab
	fi
    fi
}

do_code()
{
    if [[ -z $NOJAVA ]]
    then
	background=$1
	local jd=()
	local id=()
	local menu=()
	index=0
	# find all java source file within current directory and populate jd array with.
	for java_dir in $(find java -name '*.java' -exec dirname {} \; | sort -u)
	do
	    index=$((index+1))
	    id+=($index)
	    jd+=("$java_dir")
	    menu+=($index "$java_dir")
	done
	echo "found ${#jd[@]} java source directories"
	if [[ ${#jd[@]} -gt 1 ]]
	then
	    echo "${jd[@]}"
	    # menu <text> <height> <width> <listheight> <tag[1]> <name[1]> ... <tag[n]> <name[n]>
	    index_dir=$($DIALOG --menu "Select dir" 20 100 10 ${menu[@]} 3>&1 1>&2 2>&3)
	    if [[ ! $? ]]
	    then
		echo "[ERROR] $DIALOG --menu did fail when requesting choice over java directories" >&2
		exit 1
	    fi
	fi
	java_dir=${jd[$((index_dir-1))]}
	if [[ ! -d $java_dir ]]
	then
	    echo "$java_dir is not a directory" >&2
	    return
	fi
	echo "chose dir $java_dir"
	find $java_dir -maxdepth 1 -name '*.java' |
	    {
		local s=()
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
		    if [[ $background == codebg ]]
		    then
			${EDITOR} $java_dir/$javaclass.java &
		    else
			${EDITOR} $java_dir/$javaclass.java
		    fi
		fi
	    }
    else
	echo "NOJAVA set"
    fi
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



# WARNING can call itself.
if [[ -z "$1" ]]
then
    devenv_setup
fi
   

possible_console_gui="whiptail dialog"

for DIALOG in $possible_console_gui
do
    DIALOG=$(which $DIALOG)
    if [[ -x $DIALOG ]]
    then
	break
    fi
done

if [[ -z $DIALOG ]]
then
    echo "[ERROR] no console gui support (no dialog tool found within $possible_console_gui)  => no menus " >&2
    exit 1
fi

possible_editor="emacs vi nano"

for EDITOR in $possible_editor
do
    EDITOR=$(which $EDITOR)
    if [[ -x $EDITOR ]]
    then
	break
    fi
done

if [[ -z $EDITOR ]]
then
    echo "[ERROR] no editor found (within $possible_editor)  => no editing " >&2
fi

setup

action=initial

while [[ $action != quit ]]
do
    if [[ -z $NOJAVA ]]
    then
	# should be cleaned up from specific laby project targets.
	action=$($DIALOG --menu "Ultra Light IDE" 20 80 12 readme "Read me" clean "Clean All" ant "Ant build" run "Run it"  list_labs "Show Labyrinth with blender" test "Test it" code "Code" codebg "Code in background" deb "Debian package" properties "Edit Properties" create "Create a new class" emacsdevenv "Setup Emacs java bindings JDEE" info "Info" quit "Quit" 3>&1 1>&2 2>&3)
    else
	action=$($DIALOG --menu "Ultra Light IDE" 20 80 12 readme "Read me" clean "Clean All" run "Run it"  test "Test it" code "Code" codebg "Code in background" deb "Debian package" properties "Edit Properties" info "Info" quit "Quit" 3>&1 1>&2 2>&3)
    fi

    if [[ $action == run ]]
    then
	if [[ -z $NOJAVA ]]
	then
	    echo "run it"
	    {
		source ./project_params	    
		java -jar $(make -f ${JAVA_MAKEFILE} getname) "$default_args"
	    }
	else
	    make
	fi
    elif [[ $action == ant ]]
    then
	make -f ${JAVA_MAKEFILE} clean
	make -f ${JAVA_MAKEFILE}
	ant compile
    elif [[ $action == clean ]]
    then
	if [[ -z $NOJAVA ]]
	then
	    make -f ${JAVA_MAKEFILE} clean
            pushd java
	    make  clean
	    popd
	else
	    make clean
	fi
    elif [[ $action == test ]]
    then
	if [[ -z $NOJAVA ]]
	then
	    echo "test it"
	    pushd java
	    make display
	    popd
	else
	    make test
	fi
    elif [[ $action == deb ]]
    then
	make -f ${JAVA_MAKEFILE} deb
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
	make -f ${JAVA_MAKEFILE} $action
    elif [[ $action == info ]]
    then
	infos=$(mktemp)
	info >$infos
	$DIALOG --textbox $infos 40 80 scrolltext
	rm $infos
    fi
done
