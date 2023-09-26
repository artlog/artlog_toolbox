

copypartitiontonfs()
{
    #administrateur@poste04:/files/nfs/public$ sudo dd if=/dev/sda5 of=sda5ubuntuimg bs=32M status=progress

    partition=sda5
    devapart=/dev/$partition
    publicnfsdir=/files/nfs/public

    if [[ -d $publicnfsdir ]]
    then
	outimg=$publicnfsdir/${partition}ubuntuimg
	if [[ ! -e $outimg ]]
	then
	    echo "[WARNING] Please be patient it will take some hours" >&2
	    sudo sync
	    sudo dd if=$devpart of=$outimg bs=32M status=progress
	else
	    echo "[ERROR] there is already a $outimg file, move it away" >&2
	fi
    fi
}

# in /etc/fstab of server this is mounted :
# /data/images/sda5ubuntuimg /mnt/ubuntu18  ext4		  defaults,loop 0 0

# interesting https://wiki.archlinux.org/index.php/diskless_system

cat <<EOF
EOF

nbdsource=/data/images/sda5ubuntuimg



# https://www.admin-linux.fr/nbd-network-block-devices/


# administrateur@poste04:~/ldapserver/scripts$ grep NBD /boot/config-4.4.0-134-generic
#CONFIG_BLK_DEV_NBD=m


cat <<EOF
some info

nbd-client -h
nbd-client version 3.16.2
Usage: nbd-client -name|-N name host [port] nbd_device
	[-block-size|-b block size] [-timeout|-t timeout] [-swap|-s] [-sdp|-S]
	[-persist|-p] [-nofork|-n] [-systemd-mark|-m]
Or   : nbd-client -u (with same arguments as above)
Or   : nbd-client nbdX
Or   : nbd-client -d nbd_device
Or   : nbd-client -c nbd_device
Or   : nbd-client -h|--help
Or   : nbd-client -l|--list host
Or   : nbd-client -V|--version
All commands that connect to a host also take:
	[-F|-certfile certfile] [-K|-keyfile keyfile]
	[-A|-cacertfile cacertfile] [-H|-tlshostname hostname] [-x|-enable-tls]
Default value for blocksize is 1024 (recommended for ethernet)
Allowed values for blocksize are 512,1024,2048,4096
Note, that kernel 2.4.2 and older ones do not work correctly with
blocksizes other than 1024 without patches
Default value for port is 10809. Note that port must always be numeric
Bug reports and general discussion should go to nbd@other.debian.org
EOF

mount_ubuntu18()
{
    sudo modprobe nbd
    #if [[ ! -e /dev/nbd0 ]]
    #then
    sudo nbd-client -N export 192.168.1.55 /dev/nbd0
    #fi
    cd /mnt
    if [[ -d ubuntu18/ ]]
    then
	sudo mount -t ext4 /dev/nbd0 ubuntu18/
    fi
}


mount_ubuntu18
