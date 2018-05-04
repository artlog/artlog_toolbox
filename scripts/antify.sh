#!/bin/bash
#
# Generates to stdout an ant project build.xml content using some parameters from project_params
# this is used by Makefile.4.java

usage()
{
    head -n 4 $0 
}

PROJECT_PARAMS=./project_params
if [[ -f $PROJECT_PARAMS ]]
then
    source $PROJECT_PARAMS
else
    echo "[ERROR] missing $PROJECT_PARAMS " >&2
    usage
    exit 1
fi

# TODO reuse internal tsamp ${DSTAMP} ( after review i do't recall what is wanted to do here )

cat <<EOF
<project name="$project_name" default="$project_default" basedir="$project_basedir">
    <description>
        simple example build file
    </description>
  <!-- set global properties for this build -->
  <property name="src" location="java"/>
  <property name="build" location="build"/>
  <property name="dist"  location="dist"/>

  <target name="init">
    <!-- Create the time stamp -->
    <tstamp/>
    <!-- Create the build directory structure used by compile -->
    <mkdir dir="\${build}"/>
  </target>

  <target name="compile" depends="init"
        description="compile the source " >
    <!-- Compile the java code from \${src} into \${build} -->
EOF

if [[ -z $java_target ]]
then
    echo '   <javac srcdir="${src}" destdir="${build}"/>'
else
    echo "   <javac target=\"$java_target\" source=\"$java_target\" srcdir=\"\${src}\" destdir=\"\${build}\"><compilerarg value=\"-Xlint:-options\"/></javac>"
fi

cat <<EOF
  </target>

  <target name="dist" depends="compile"
        description="generate the distribution" >
    <!-- Create the distribution directory -->
    <mkdir dir="\${dist}/lib"/>

    <!-- Put everything in \${build} into the ${project_name}-${project_version}.jar file  ( \${DSTAMP} not used yet )-->
    <jar jarfile="\${dist}/lib/${project_name}-${project_version}.jar" basedir="\${build}">
            <manifest>
                <attribute name="Main-Class" value="$project_mainclass"/>
            </manifest>
     </jar>
  </target>

  <target name="clean"
        description="clean up" >
    <!-- Delete the \${build} and \${dist} directory trees -->
    <delete dir="\${build}"/>
    <delete dir="\${dist}"/>
  </target>
</project>
EOF
