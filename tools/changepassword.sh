#!/bin/bash

usage()
{
    echo "admindn= -admin user [password]"
}

# will set new_hashed_password
get_password()
{
    local admindn=$1
    local useruid=$2
    local ldaphost=$3

    echo "${admindn} ${useruid} ${ldap_host}"    
    #    ldapsearch -LLL -D ${admindn} -W -H $ldap_host $useruid
    new_hashed_password=$(ldapsearch -LLL -D ${admindn} -W -H $ldap_host '(uid=philippel)' userPassword |
	{
	    collect=""
	    while read line
	    do
		if [[ -n $collect ]]
		then
		    collect="${collect}${line}"
		else		   
		    if [[ $line =~ userPassword::(.*) ]]
		    then
			collect=${BASH_REMATCH[1]}		       
		    fi
		fi
	    done
	    echo $collect
	})
	
}

resolve_basedn()
{
    if [[ -z $basedn ]]
    then
	basedn="dc=slv-valbonne,dc=fr"
    fi

}

resolve_userdn()
{
    if [[ -z $userdn ]]
    then
	resolve_basedn
	if [[ $basedn =~ yunohost ]]       
	then
	    userdn="uid=${useruid},ou=users,${basedn}"
	else
	    userdn="uid=${useruid},ou=people,${basedn}"
	fi
    fi
}


resolve_admindn()
{
    resolve_basedn
    
    if [[ -z $admindn ]]
    then
	if [[ -n $admin ]]
	then
	    admindn="cn=${admin},${basedn}"
	else
	    resolve_userdn
	    admindn="$userdn"
	fi
    fi
}

resolv_ldap_host()
{
    if [[ -z $ldap_host ]]
    then
	ldap_host=ldaps://ldaps.slv-valbonne.fr
    fi
}

show_variables()
{
    echo "
	    useruid=${userid}
            userdn=${userdn}
	    admin=${admin}
	    ldap_host=${ldap_host}
	    admindn=${admindn}
	    basedn=${basedn}"
}

while [[ $# > 0 ]]
do
    case $1 in
	admindn=*)
	    admindn=${1/admindn=/}
	    ;;
	-admin)
	    admin='admin'
	    ;;
	uri=*)
	    ldap_host=${1/uri=/}
	    ;;
	base=*)
	    basedn=${1/base=/}
	    ;;
	-fetch)
	    if [[ -z $useruid ]]
	    then
		echo "[ERROR] fetch should be after userudi"
	    fi
	    resolve_admindn
	    get_password $admindn $userid $ldaphost
	    if [[ -z $new_hashed_password ]]
	    then
		echo "[ERROR] can't get hashed_password" >&2
		exit 1
	    fi
	    echo "new_hashed_password : ${new_hashed_password}"
	    echo "reset useruid admindn basedn and ldap_host"
	    useruid=
	    userdn=
	    admin=
	    ldap_host=
	    admindn=
	    basedn=
	    ;;	
	*)
	    if [[ -z $useruid ]]
	    then
		useruid=$1
	    else
		if [[ -z $new_hashed_password ]]
		then
		    
		    new_hashed_password="$1"
		    echo "[WARNING] resetting directly with a hashed password '$1'" >&2
		else
		    echo "[ERROR] unexpected extra argument '$1'"
		    exit 1
		fi
	    fi
    esac		  
    shift 1
done


show_variables

if [[ -z $useruid ]]
then
    usage
    exit 1
fi  

if [[ -z $new_hashed_password ]]
then
    echo "Please enter new password for ${useruid}"
    new_hashed_password=$(slappasswd -h '{SSHA}')
    new_hashed_password=$(echo "${new_hashed_password}" | openssl base64 -d)
fi

resolve_userdn
resolve_admindn

echo "..."

cat <<EOF >newpassword.ldif
dn: ${userdn}
changetype: modify
replace: userPassword
userPassword:: ${new_hashed_password}
EOF

cat newpassword.ldif

echo "..."

echo "change password for $userdn on $ldap_host using credentials of ${admindn}"
ldapmodify -c -D ${admindn} -W -a -f newpassword.ldif -H $ldap_host

rm newpassword.ldif
