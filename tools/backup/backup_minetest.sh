#!/bin/bash

# date +%s == epoch
# timesstamp ex : 2020-09-01-08-45-09
timestamp=$(date "+%F-%H-%M-%S")
SUDO=sudo

minetest_worlds_dir=/home/yunohost.app/minetest/.minetest/worlds
minetest_world=world
backup_dir=/home/admin/backup/minetest

echo "[WARNING] stopping minetest to take a backup"
$SUDO systemctl stop minetest
backup_dest=$backup_dir/world.$timestamp.tgz
echo "[INFO] create an archive of '$minetest_world' world within $minetest_worlds_dir path to $backup_dest"
$SUDO tar -czf $backup_dest -C $minetest_worlds_dir $minetest_world
$SUDO systemctl start minetest
echo "[INFO] minetest restarted"
