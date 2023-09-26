#!/bin/bash



while [[ $# > 0 ]]
do
    case $1 in
	entity=*)
	    entity=${1/entity=/}
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

# delegate to specific scripts.

source ./createcacredentials.sh

source ./createldapscredentials.sh
