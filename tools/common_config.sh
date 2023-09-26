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
    if [[ -z $webserver ]]
    then
	webserver=webserver
	log_any "[WARNING] webserver undefined, using $webserver"
    fi

    if [[ -z $entity ]]
    then
	log_error "entity undefined, Please set entity="
	exit 1
    fi

    cakeyfile=${entity}-ca.key
    cacertfile=${entity}-ca-cert.pem
    ldapskeyfile=${entity}-ldaps.key
    ldapsreqfile=${entity}-ldaps-cert.req
    ldapscertfile=${entity}-ldaps-cert.pem
    webserverkeyfile=${entity}-${webserver}.key
    webserverreqfile=${entity}-${webserver}-cert.req
    webservercertfile=${entity}-${webserver}-cert.pem

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
}
