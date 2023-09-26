#!/bin/bash

# http://techiezone.rottigni.net/2011/12/change-root-dn-password-on-openldap/

ldapbasedn="dc=slv-valbonne,dc=fr"
admindn="cn=admin,${ldapbasedn}"

sudo ldapsearch -LLL -Y EXTERNAL -H ldapi:/// -b  cn=config olcRootDN=${admindn} dn olcRootDN olcRootPW >ldappreviouspass.txt

sudo ldapsearch -LLL -Y EXTERNAL -H ldapi:/// -b  cn=config olcRootDN=${admindn} dn >ldapbackend.txt

slappasswd -h {SSHA} >ldappassword.txt

{
    head -n1 ldapbackend.txt
    echo "replace: olcRootPW"
    echo -n "olcRootPW: "
    cat ldappassword.txt
} >modifyldappassword.ldif

cat modifyldappassword.ldif

sudo ldapmodify -Y EXTERNAL -H ldapi:/// -f modifyldappassword.ldif
