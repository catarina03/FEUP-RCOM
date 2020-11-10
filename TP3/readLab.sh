#!/bin/bash

rm read_obj

#clear

if gcc -o read_obj -Wall interface.c application.c supervision.c alarme.c parseNbuild.c printers.c; then
echo "---------"
./read_obj -p /dev/ttyS0 -r .. ;
else
echo "COMPILATION ERROR";
fi