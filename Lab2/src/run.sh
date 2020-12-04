#!/bin/bash

#rm download

if gcc -o download -Wall download.c; then
./download "ftp://[<user>:<password>@]<host>/<url-path>" 
else
echo "COMPILATION ERROR";
fi

# ./download "ftp://[<user>:<password>@]<host>/<url-path>" 
# ./download "ftp://<host>/<url-path>" 

