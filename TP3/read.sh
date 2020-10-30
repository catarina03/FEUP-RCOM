#!/bin/bash

rm read_obj

#clear

if gcc -o read_obj -Wall interface.c application.c alarme.c noncanonical.c writenoncanonical.c protocol.c; then
echo "---------"
./read_obj -p /dev/ttyS11 -r . ;
else 
echo "COMPILATION ERROR";
fi 
