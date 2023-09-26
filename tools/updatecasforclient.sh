#!/bin/bash

cacertfile=/files/nfs/users/cyberadmin/slv-valbonne-ca-cert.pem

caconf=/etc/ca-certificates.conf
casourcedir=/usr/share/ca-certificates/

if [[ ! -d $casourcedir/extra/ ]]
then
    sudo mkdir -p $casourcedir/extra/
fi
sudo cp ${cacertfile} $casourcedir/extra/

cacertbasefile=$(basename ${cacertfile})

if grep extra/${cacertbasefile} $caconf
then
    echo "certificate ${cacertbasefile}  found in $caconf"
else
    echo "certificate ${cacertbasefile} not found in $caconf"

    echo "[WARNING] adding ${cacertbasefile} in  $caconf"
    sudo bash -c "echo 'extra/${cacertbasefile}' >> $caconf "
fi

sudo update-ca-certificates

# https://askubuntu.com/questions/244582/add-certificate-authorities-system-wide-on-firefox

nsscerttool=certutil
if which $nsscerttool
then
    certificateName="SLV VALBONNE"
    # skipping  ~/.thunderbird ..; cert8
    for certDB in $(find  ~/.mozilla* -name "cert9.db")
    do
	certDir=$(dirname ${certDB});
	echo "[INFO] mozilla certificate install '${certificateName}' from file  ${cacertfile} in ${certDir}" >&2
	# -d sql: fro cert9, dbm for cert8 see https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/Tools/certutil
	$nsscerttool -A -n "${certificateName}" -t "TCu,Cuw,Tuw" -i ${cacertfile} -d sql:${certDir}
    done

    echo "[INFO] please restart firefox to get new certificates in action"
else
    echo "[ERROR] $nsscerttool tool not found" >&2
    echo " You may find certutil in the libnss3-tools package (debian/ubuntu)."
fi
