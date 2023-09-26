#!/bin/bash

createuser=$1

if [[ -z $createuser ]]
then
    echo "[ERROR] expect a non existing home for user like plhardy" >&2
    exit 1
fi

sudo mkhomedir_helper $createuser

sudo -u $createuser ln -s /files/nfs/users/$createuser /home/${createuser}/nfs

