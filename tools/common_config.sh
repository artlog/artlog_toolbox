log_any()
{
    echo $@ >&2
}

log_error()
{
    log_any '[ERROR]' $@
}

setup_certificate_configurations()
{

    if [[ -z $entity ]]
    then
	log_error "entity undefined, Please set entity="
	exit 1
    fi

    cakeyfile=${entity}-ca.key
    cacertfile=${entity}-ca-cert.pem

    if [[ -z $ca ]]
    then
	if [[ ! -f $cakeyfile ]]
	then
	    log_error "missing $cakeyfile for entity=$entity CAN'T sign certificates"
	    exit 1
	fi
    fi

    if [[ -n $ldap ]]
    then
	prefix=${entity}-${ldap}
	ldapskeyfile=${prefix}.key
	ldapsreqfile=${prefix}-cert.req
	ldapscertfile=${prefix}-cert.pem
    fi

    if [[ -n $webserver ]]
    then
	prefix=${entity}-${webserver}
	webserverkeyfile=${prefix}.key
	webserverreqfile=${prefix}-cert.req
	webservercertfile=${prefix}-cert.pem
    fi

    if [[ -n $client ]]
    then
	prefix=${entity}-user-${client}
	clientkeyfile=${prefix}.key
	clientreqfile=${prefix}-cert.req
	clientcertfile=${prefix}-cert.pem
    fi

    dcvalue=${entity}.fr
    webserverdnsname=${webserver}.${entity}.fr

    echo "# certificates files config"

    echo "cakeyfile=$cakeyfile"
    echo "cacertfile=$cacertfile"
    echo "ldapskeyfile=$ldapskeyfile"
    echo "ldapsreqfile=$ldapsreqfile"
    echo "ldapscertfile=$ldapscertfile"
    echo "webserverkeyfile=$webserverkeyfile"
    echo "webserverreqfile=$webserverreqfile"
    echo "webservercertfile=$webservercertfile"
    echo "dcvalue=$dcvalue"
    echo "webserverdnsname=$webserverdnsname"
    echo "clientkeyfile=$clientkeyfile"
    echo "clientreqfile=$clientreqfile"
    echo "clientcertfile=$clientcertfile"

}
