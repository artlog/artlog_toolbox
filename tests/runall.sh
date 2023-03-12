#!/bin/bash

pushd ../c
make ../build/base64_dbg
gcc -g -O0 -DDEBUG_BASE64 -DDEBUG -o ../build/base64_dbg albase64url_main.c alstrings.c aldebug.c altodo.c alinput.c albase64.c albitfieldreader.c aloutput.c albitfieldwriter.c al_options.c -L../build/lib -Wl,-Bstatic -lalhash  -Wl,-Bdynamic
popd

for project in abnf base64 cbor hashtree json lib
do
    pushd $project
    ./test1.sh
    popd
done
