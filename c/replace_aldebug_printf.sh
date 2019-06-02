#/!bin/bash

filein=$1

if [[ -f $filein ]]
then
    sed -i "s/aldebug_printf(NULL,/aldebug_printf(DBGSTREAM,/g" $filein
fi
