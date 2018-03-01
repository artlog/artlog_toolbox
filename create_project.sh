#!/bin/bash
# see usage()

function usage()
{
    echo "create a project in parent directory named after project name"
    echo "$0 <project_name> c|java"
}

function check_dependencies()
{
    if [[ -x $(which git) ]]
    then
	echo "[INFO] git found"
    else
	echo "[ERROR] git tool is needed " >&2
	exit 1
    fi
}
function create_makefile_for_java()
{
    cat <<EOF >java/Makefile
PACKAGE=org.artisanlogiciel.$project_name
PACKAGE_DIR=\$(subst .,/,\$(PACKAGE))
OUT=out
EDITOR=emacs

\$(OUT):
	mkdir -p \$(OUT)

clean:
	@find \$(PACKAGE_DIR) -name "*.class" -type f -print0|xargs -0 rm 2>/dev/null && echo "cleaned classes in source"
	@find \$(OUT) -name "*.class" -type f -print0|xargs -0 rm 2>/dev/null || echo "nothing to clean"

test:
	javac -d \$(OUT) \$(PACKAGE_DIR)/Main.java
	java -cp \$(OUT) \$(PACKAGE).Main


run/%:	\$(OUT)
	javac -d \$(OUT) \$(PACKAGE_DIR)/\$(subst run/,,\$@).java
	java -cp \$(OUT) \$(PACKAGE)/\$(subst run/,,\$@) 

compile/%:
	javac -d \$(OUT) \$(PACKAGE_DIR)/\$(subst compile/,,\$@).java

\$(PACKAGE_DIR)/%.java:
	./generate_new.sh class \$(subst .java,,\$(subst \$(PACKAGE_DIR)/,,\$@))

interface/%:
	./generate_new.sh interface package_dir=\$(PACKAGE_DIR) \$(subst interface/,,\$@)
	\$(EDITOR) \$(PACKAGE_DIR)/\$(subst interface/,,\$@).java

work/%:	\$(PACKAGE_DIR)/\$(subst work/,,%).java
	\$(EDITOR) \$<

work:	work/LabyModel

save:	
	git citool

.PHONY: clean test work work/% run/% save compile/% interface/%

# tried to avoid intermediate file removal : does not work
# .SECONDARY: \$(PACKAGE_DIR)/%.java 

# this does work : once precious intermediate file is not removed.
.PRECIOUS: \$(PACKAGE_DIR)/%.java 
EOF
}

function create_makefile_for_c()
{

    binary=$project_name
    
cat <<EOF >c/Makefile

CC=gcc
LD=gcc
CPPFLAGS=-g

BUILD=build

src=main.c

objects=\$(patsubst %.c,\$(BUILD)/obj/%.o,\$(src))

\$(objects): | \$(BUILD)/obj

test:\$(BUILD)/binary
	mkdir -p \$@
	./\$(BUILD)/binary test.binary >test/parse1.binary
	./\$(BUILD)/binary parse1.binary >test/parse2.binary
	diff test/parse1.binary test/parse2.binary

\$(BUILD)/binary: \$(objects)
	@echo link objects \$(objects)
	\$(LD) \$(LDFLAGS) \$^ -o \$@

\$(BUILD)/obj:
	mkdir -p \$@

\$(BUILD)/obj/%.o: %.c
	@echo compile \$< 
	@\$(CC) -c \$(CFLAGS) \$(CPPFLAGS) \$< -o \$@

clean:
	rm -f \$(objects)

.PHONY:clean test
EOF
}

function create_makefile_for()
{
    l=$1
    if [[ "$l" == "java" ]]
    then
	create_makefile_for_java
    elif [[ "$l" == "c" ]]
    then
	create_makefile_for_c
    fi
}

if [[ $# == 0 ]]
then
    usage
    exit 1
fi

THIS_TOOL=$0
ARTLOG_TOOLBOX=$(dirname $(pwd)/$THIS_TOOL)

locatetoolbox=./scripts/locate_artlog_toolbox.sh
if [[ -f $locatetoolbox ]]
then
    source $locatetoolbox
else
    echo "[EROOR] missing $locatetoolbox" >&2
    usage
    exit 1
fi

select_artlog_toolbox artlog_toolbox

echo "Using ARTLOG_TOOLBOX=$ARTLOG_TOOLBOX"

check_dependencies

project_name=$1

shift

while [[ $# > 0 ]]
do
    case $1 in
	c|java|python)
	    languages="$languages $1"
	    ;;
	*)
	    echo "[ERROR] Unrecognized $1 option" >&2
	    exit 1
    esac
    shift
done

if [[ -z $languages ]]
then
    echo "[WARNING] no language given, default to c" >&2
    languages=c
fi

pushd .. >/dev/null
if [[ -d $project_name ]]
then
    echo "[ERROR] project $project_name already exists in $(pwd)" >&2
    exit 1
fi
echo "[INFO] Creating project $project_name within $(pwd)"
mkdir $project_name
if [[ ! -d $project_name ]]
then
    echo "[ERROR] creationg of $project_name directory failed" >&2
    exit 1
fi

cp $ARTLOG_TOOLBOX/scripts/locate_artlog_toolbox.sh $project_name/init.sh

cat <<EOF >>$project_name/init.sh

# AFTER function copied from $ARTLOG_TOOLBOX/scripts/locate_artlog_toolbox.sh

if [[ -z \$ARTLOG_TOOLBOX ]]
then
   ARTLOG_TOOLBOX=\$(pwd)/artlog_toolbox
   select_artlog_toolbox artlog_toolbox
   echo "no ARTLOG_TOOLBOX found, use a dedicated one \$ARTLOG_TOOLBOX"
fi


if [[ ! -d \$ARTLOG_TOOLBOX ]]
then
    git clone https://github.com/artlog/artlog_toolbox.git \$ARTLOG_TOOLBOX
    if [[ -d \$ARTLOG_TOOLBOX ]]
    then
      pushd \$ARTLOG_TOOLBOX
      git checkout master
      popd
    else
      echo "[ERROR] git creation of \$ARTLOG_TOOLBOX failed" >&2
      exit 1
    fi
fi
echo "ARTLOG_TOOLBOX=\$ARTLOG_TOOLBOX" >toolbox.param
\$ARTLOG_TOOLBOX/deploy.sh
EOF

pushd $project_name >/dev/null
chmod u+x init.sh
if [[ ! -x ./init.sh ]]
then
    echo "[ERROR] setup failure, init.sh is not executable" >&2
    exit 1
fi
echo "to setup project $project_name :"
echo "cd $(pwd); ./init.sh"

git init
git add ./init.sh
for l in $languages
do
    mkdir $l
    echo "source placeholder for $project_name in $l autogenerated by $0 $(date)" > $l/README
    git add $l/README
    create_makefile_for $l
    if [[ -f $l/Makefile ]]
    then
	git add $l/Makefile
    fi
    if [[ "$l" = "java" ]]
    then
	cat <<EOF >project_param
project_name=$project_name
project_default=dist
project_basedir=\$(pwd)
project_mainpackage=org.artisanlogiciel.$project_name
project_mainclass=\${project_mainpackage}.Main
project_version=0.0.1
default_args=''
EOF
    fi
done

# FIXME for javascript
if [[ $languages =~ java ]]
then
    touch devenv_params
else
    echo "[INFO] no java"
    echo "NOJAVA='c only'" >devenv_params
fi
for param_file in devenv_params project_params
do
    if [[ -e $param_file ]]
    then
	git add $param_file
    fi
done
cat <<EOF >README.md
Newly create $project_name by $0

Please enter here a correct explanation for this project

./doit.sh
EOF
git add README.md

ARTLOG_TOOLBOX=$ARTLOG_TOOLBOX ./init.sh

git commit -m "initial commit project $project_name created by artlog toolbox"

popd >/dev/null
popd >/dev/null
       
