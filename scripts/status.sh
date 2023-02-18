#!/bin/bash

echo "=== lxc ==="

sudo  lxc list

echo "=== docker ==="

sudo docker image list

echo "=== teamviewer ==="

sudo systemctl status teamviewerd
