#!/bin/bash

check_config_files_exist()
{
    for credfile in $cakeyfile $cacertfile $ldapskeyfile $ldapsreqfile $ldapscertfile
    do
	if [[ ! -f $credfile ]]
	then
	    echo "[WARNING] Missing $credfile" >&2
	fi
    done
}

# input etcssldir
# input etccassldir
copycredtoetcssl()
{
    
    if [[ ! -d $etcssldir ]]
    then
	echo "[ERROR] $etssldir missing" >&2
    fi
    # fixme
    if [[ ! -e ${etcssldir}/certs/$(basename $cacertfile) ]]
    then
	echo "[WARNING] copying/forcing $cacertfile into  ${etcssldir}/certs/ ">&2
	sudo cp $cacertfile  ${etcssldir}/certs/
    fi
    if [[ ! -d ${etcldapssldir} ]]
    then
	sudo mkdir -p ${etcldapssldir}
    fi
    sudo cp $ldapscertfile $ldapskeyfile ${etcldapssldir}/
    sudo chown -R openldap:openldap ${etcldapssldir}/
}

# input etcssldir
# input etccassldir
addolcTLSCertificates()
{

    ldifaddolcTLSCertificates=addolcTLSCertificates.ldif
    cat <<EOF >$ldifaddolcTLSCertificates
dn: cn=config
add: olcTLSCACertificateFile
olcTLSCACertificateFile: ${etcssldir}/certs/$cacertfile
-
add: olcTLSCertificateFile
olcTLSCertificateFile: ${etcldapssldir}/$ldapscertfile
-
add: olcTLSCertificateKeyFile
olcTLSCertificateKeyFile: ${etcldapssldir}/$ldapskeyfile
EOF

    cat $ldifaddolcTLSCertificates

    # need sudo here for ldapi:/// local access
    sudo ldapmodify -Y EXTERNAL -H ldapi:/// -f $ldifaddolcTLSCertificates
}

common_config=common_config.sh
if [[ ! -f $common_config ]]
then
    echo"[ERROR] Missing $common_config" >&2
    exit 1
fi

source common_config.sh

check_config_files_exist

etcssldir=/etc/ssl
etcldapssldir=/etc/ldap/ssl

copycredtoetcssl

addolcTLSCertificates
