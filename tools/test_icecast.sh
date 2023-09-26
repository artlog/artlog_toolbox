#!/bin/bash

parse_args()
{
    while [[ $# > 0 ]]
    do
	case $1 in
	    launch_mode=*)		
		# launch_mode manual or scheduled
		# scheduled means that it should run now and was triggered by a scheduler ( systemd or cron or aother ... )
		launch_mode=${1//launch_mode=/}
		;;
	    server_address=*)
		server_address=${1//server_address=/}
		;;
	    source_dir=*)
		source_dir=${1//source_dir=/}
		;;
	    slvtv_user=*)
		slvtv_user=${1//slvtv_user=/}
		;;
	    icecast_client_port=*)
		icecast_client_port=${1//icecast_client_port=/}
		;;
	    *)
		echo "[WARNING] argument '$1' ignored" >&2
	esac
	shift
    done
}

conf_dir=~/.svtv
status_file=$conf_dir/status.tmp

if [[ ! -d $conf_dir ]]
then
    echo "[INFO] creating $conf_dir"
    mkdir -p $conf_dir
fi

# status of other broadcasts for this configuration
# protect again mutliple scheduled brodacast
broadcast_status=unknown

if [[ -f $status_file ]]
then
    broadcast_status=$(grep "broadcast_status=" $status_file)    
else
    echo "[INFO] creating $status_file"
    touch $status_file    
fi

launch_mode=manual
server_address=192.168.1.64
slvtv_user=cyberadminserver
slvtv_home=~$slvtv_user
source_dir=$slvtv_home
icecast_client_port=8066

parse_args $@

echo "launch_mode=$launch_mode"
echo "server_address=$server_address"
echo "source_dir=$source_dir"

if [[ $launch_mode == manual ]]
then
    echo "manual_mode"
    if [[ $broadcast_status == manual ]]
    then
	echo "[WARNING] another manual broadcast is running" >&2
    else	
	echo "broadcast_status=manual" >> $status_file
    fi
else
    if [[ $broadcast_status == manual ]]
    then
	echo "[ERROR] a broadcast is already manualy set broadcast_status=$broadcast_status" >&2
    fi
fi

SERVERADDRESS=$server_address
SOURCEDIRECTORY=$source_dir


if [[ -d $SOURCEDIRECTORY ]]
then
    pushd $SOURCEDIRECTORY
    MOVIEDIR=slvtv

    if [[ -d $MOVIEDIR ]]
    then
	pushd $MOVIEDIR

	for ogg_video in *.og[gv]
	do
	    ffmpeg -i $ogg_video -f ogg -content_type video/ogg -qscale:v 10 -qscale:a 10 icecast://source:NotreTvANousSlv@${SERVERADDRESS}:$icecast_client_port/slvtv.ogg
	done

	popd
    else
	echo "[ERROR] expecting a slvtv directory here in $(pwd)" >&2
    fi
    popd
else
    echo "[ERROR] missing source_dir=$SOURCEDIRECTORY" >&2
fi

if [[ $launch_mode == manual ]]
then
    echo "[WARNING] in manual case remove status file at end"
    rm  $status_file
fi
