#!/bin/bash

check_config_files_exist()
{
    for credfile in $cakeyfile $cacertfile $webserverkeyfile $webserverreqfile $webservercertfile
    do
	if [[ ! -f $credfile ]]
	then
	    echo "[WARNING] Missing $credfile" >&2
	fi
    done
}

# input etcssldir
copywebservercredtoetcssl()
{
    
    if [[ ! -d $etcssldir ]]
    then
	echo "[ERROR] $etssldir missing" >&2
    fi
    sudo cp $cacertfile $webservercertfile ${etcssldir}/certs/
    sudo cp $webserverkeyfile ${etcssldir}/private/
}

# output
# webserver_apache.conf
createapachetlsconf()
{
    echo "TODO create apache tls conf"

    local tlscacert=${etcssldir}/certs/$(basename $cacertfile)
    local tlscert=${etcssldir}/certs/$(basename $webservercertfile)
    local tlskey=${etcssldir}/private/$(basename $webserverkeyfile)

    cat <<EOF >webserver_apache.conf
       		SSLEngine on

		SSLCertificateFile	$tlscert
		SSLCertificateKeyFile	$tlskey
		SSLCACertificateFile	$tlscacert
EOF
		
}

common_config=common_config.sh
if [[ ! -f $common_config ]]
then
    echo"[ERROR] Missing $common_config" >&2
    exit 1
fi

source $common_config

check_config_files_exist

etcssldir=/etc/ssl

copywebservercredtoetcssl

createapachetlsconf
