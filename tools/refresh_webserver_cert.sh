#!/bin/bash

./createwebservercredentials.sh
./setupwebserverapache.sh

sudo service apache2 restart
