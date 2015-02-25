#!/bin/bash

action=$1
file=$2

maintainer="Artlog <pl@artisanlogiciel.net>"

if [[ $action == getversion ]]
then
    source ./project_params
    echo $project_version
elif [[ $action == create ]]
then
    source ./project_params

    if [[ $file == debian/control ]]
    then
    
    cat <<EOF
Source: ${project_name}-java
Priority: extra
Maintainer: $maintainer
Build-Depends: debhelper (>= 7.0.50~), javahelper (>=0.25)
Build-Depends-Indep: default-jdk, default-jdk-doc, libtrove-java, libtrove-java-doc, ant
Standards-Version: ${project_version}
Section: java

Package: ${project_name}-java
Architecture: all
Depends: \${java:Depends}, \${misc:Depends}
Recommends: \${java:Recommends}
Description: Labyrinth generator
  Labyrinth generator

Package: ${project_name}-java-doc
Architecture: all
Depends: \${java:Depends}, \${misc:Depends}
Recommends: \${java:Recommends}
Description: Labyrinth generator (documentation)
 Labyrinth generator
EOF
    elif [[ $file = debian/changelog ]]
    then
    cat <<EOF
${project_name}-java (${project_version}) unstable; urgency=low

  * Initial debian package

 -- ${maintainer}  $(LANG=C date -R)
EOF
    elif [[ $file = debian/rules ]]
    then
	cat <<EOF
#!/usr/bin/make -f
JAVA_HOME=/usr/lib/jvm/default-java

%:
	dh \$@ --with javahelper
EOF
    elif [[ $file = deb/javadoc ]]
    then
	echo "api /usr/share/doc/${project_name}-java/api" >debian/${project_name}-java-doc.javadoc
    elif [[ $file = deb/jlibs ]]
    then
	echo "dist/lib/${project_name}-${project_version}.jar" >debian/${project_name}-java.jlibs
    fi
fi
