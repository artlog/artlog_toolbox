#!/bin/bash
# create a project in parent directroy named after project name

project_name=$1

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
pushd $project_name >/dev/null
cat <<EOF >init.sh
#!/bin/bash

if [[ ! -d  artlog_toolbox ]]
then
    git clone https://github.com/artlog/artlog_toolbox.git artlog_toolbox
    pushd artlog_toolbox
    git checkout master
    popd
fi
artlog_toolbox/deploy.sh
EOF

chmod u+x init.sh
if [[ ! -x ./init.sh ]]
then
    echo "[ERROR] setup failure, init.sh is not executable" >&2
    exit 1
fi
echo "to setup project $project_name :"
echo "cd $(pwd); ./init.sh"

popd >/dev/null
popd >/dev/null
       
