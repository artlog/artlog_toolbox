

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
apt install nbd-server

Creating config file /etc/nbd-server/config with new version
Ajout de l'utilisateur système « nbd » (UID 126) ...
Ajout du nouveau groupe « nbd » (GID 129) ...
Ajout du nouvel utilisateur « nbd » (UID 126) avec pour groupe d'appartenance « nbd » ...
Le répertoire personnel « /etc/nbd-server » n'a pas été créé.


root@cyberserveurinterne:/data/images# grep nbd /etc/services 
nbd		10809/tcp			# Linux Network Block Device
enbd-cstatd	5051/tcp			# ENBD client statd
enbd-sstatd	5052/tcp			# ENBD server statd

EOF


# /data/images/sda5ubuntuimg

nbdexportconf=nbdexport.conf

cat <<EOF >$nbdexportconf
[export]
	exportname = /data/images/sda5ubuntuimg
	readonly = true
	copyonwrite = false
EOF

echo "[INFO] copy $nbdexportconf file into /etc/nbd-server/conf.d/ directory"

sudo cp nbdexport.conf /etc/nbd-server/conf.d/
