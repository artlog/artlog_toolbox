#!/bin/bash

state="Bretagne"
locality="Liffre"

while [[ $# > 0 ]]
do
    case $1 in
	webserver=*)
	    webserver=${1/webserver=/}
	    ;;
	ldapserver=*)
	    ldapserver=${1/ldapserver=/}
	    ldap="$ldapserver"
	    ;;
	ldap=*)
	    ldap=${1/ldap=/}
	    ldapserver="$ldap"
	    ;;
	ca=*)
	    ca=${1/ca=/}
	    ;;
	client=*)
	    client=${1/client=/}
	    ;;
	entity=*)
	    entity=${1/entity=/}
	    ;;
	locality=*)
	    locality=${1/locality=/}
	    ;;
	state=*)
	    state=${1/state=/}
	    ;;
	dryrun)
	    defer=echo
	    ;;
	*)
	    log_error "Unrecognized entity"
	    ;;
    esac
    shift
done

# create ca authority and ldaps certificate

# https://www.gnutls.org/manual/html_node/certtool-Invocation.html

common_config=common_config.sh
if [[ ! -f $common_config ]]
then
    echo"[ERROR] Missing $common_config" >&2
    exit 1
fi
source $common_config

setup_certificate_configurations

organisation=$entity

# delegate to specific scripts.

[[ -n $ca ]] && source ./createcacredentials.sh

[[ -n $ldapserver ]] && source ./createldapscredentials.sh

[[ -n $webserver ]] && source ./createsubwebservercredentials.sh

[[ -n $client ]] && source ./createclientcredentials.sh
