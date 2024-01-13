#!/bin/bash


source ./check_kernel.sh

# open file browser
xdg-open ~&

echo "Please follow instructions opened in emacs."
echo "Will continue once emacs is closed"

emacs startday.md

firefox&

echo "Dans keepassxc 'philippe.lhardy ldap'"

keepassxc&

./daily.sh

