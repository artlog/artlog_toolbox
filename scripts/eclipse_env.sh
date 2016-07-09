#!/bin/bash
# for the story this comes from a JavaFX training i attended

usage()
{
    echo "Usage $0:"
    echo "setup_dev_env: create file devenv_params with reference over jdk and eclipse path"
    echo "any other argument is an error"
    echo "no argument => launch eclipse"
}
create_dev_params()
{
    if [[ -e devenv_params ]]
    then
	echo " devenv_params already exists"
	exit 1
	# could update this way
	# source devenv_params
    else
	if [[ -z $DEV_ENV ]]
	then
	    echo "[ERROR] DEV_ENV $DEV_ENV not set, call setup_eclipse_dir from this '$0' script"
	    exit 1
	fi
	cat >devenv_params <<EOF
DEV_ENV=$DEV_ENV
JDK_PATH=$JDK_PATH
ECLIPSE_PATH=$ECLIPSE_PATH
EOF
    fi
}

setup_eclipse_dir()
{
    if [[ -e devenv_params ]]
    then
	source devenv_params
    else
	DEV_ENV=~/artisanlogiciel/devel_tools
	JDK_PATH=${DEV_ENV}/jdk1.8.0_74
	ECLIPSE_PATH=${DEV_ENV}/eclipse
    fi
}

# Not tested written after install ...
setup_eclipse()
{
    # current list of TGZ got from oracle (tm)
    TGZ_LIST="eclipse-jee-mars-2-linux-gtk-x86_64.tar.gz javafx_scenebuilder-2_0-linux-x64.tar.gz jdk-8u74-linux-x64.tar.gz"   
}

check_eclipse()
{
    if [[ -z $JDK_PATH ]]
    then
	echo "[ERROR] Missing JDK_PATH, either devenv_params is wrong or missing" >&2
	exit 1
    fi
    if [[ -z $ECLIPSE_PATH ]]
    then
	echo "[ERROR] Missing ECLIPSE_PATH, either devenv_params is wrong or missing" >&2
	exit 1
    fi
    if [[ ! -d $JDK_PATH ]]
    then
	echo "[ERROR] Missing JDK_PATH '$JDK_PATH'" >&2
    fi
    
    if [[ ! -d $ECLIPSE_PATH ]]
    then
	ls -la ${DEV_ENV}
	
	echo "[ERROR] Missing ECLIPSE_PATH '$ECLIPSE_PATH'" >&2
	exit 1
    fi

}

# not yet used ...
create_sandbox()
{
    mkdir sandbox
}

# overrides PATH with JDK_PATH ECLIPSE_PATH
# and check java is the right one to launch eclipse
run_in_sandbox()
{
    PATH=$JDK_PATH/bin/:$JDK_PATH/jre/bin/:$ECLIPSE_PATH:$PATH

    export PATH

    if java -version 2>&1 | grep '1.8.0'
    then
	if javac -version 2>&1 | grep '1.8.0'
	then
	    $*
	else
	    echo "[ERROR] Wrong javac  version" >&2
	fi
    else
	echo "[ERROR] Wrong java  version" >&2
    fi
}

while [[ $# > 0 ]]
do
    case $1 in
	setup_dev_env)
	    setup_eclipse_dir
	    create_dev_params
	    exit 0
	    ;;
	*)
	    echo "[ERROR] Unrecognized argument '$1'" >&2
	    usage
	    exit 1
	    ;;
    esac
    shift 1
done

setup_eclipse_dir
check_eclipse
run_in_sandbox eclipse -data $(pwd)/workspace

