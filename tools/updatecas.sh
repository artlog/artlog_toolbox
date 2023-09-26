#!/bin/bash

common_config=common_config.sh
if [[ ! -f $common_config ]]
then
    echo"[ERROR] Missing $common_config" >&2
    exit 1
fi

source $common_config

caconf=/etc/ca-certificates.conf
casourcedir=/usr/share/ca-certificates/

sudo cp ${cacertfile} $casourcedir/extra/

if grep extra/${cacertfile} $caconf
then
    echo "certificate found in $caconf"
else
    echo "certificate not found in $caconf"
fi

sudo update-ca-certificates
