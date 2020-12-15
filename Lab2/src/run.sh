#!/bin/bash

#rm download

if gcc -o download -Wall download.c; then
./download ftp://ftp.up.pt/pub/gnu/GNUinfo/Audio/index.txt;
else
echo "COMPILATION ERROR";
fi

# ./download "ftp://[<user>:<password>@]<host>/<url-path>" 
# ./download "ftp://<host>/<url-path>" 

