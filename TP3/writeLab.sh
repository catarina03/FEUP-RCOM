#!/bin/bash

rm write_obj

#clear

if gcc -o write_obj -Wall interface.c application.c supervision.c alarme.c parseNbuild.c printers.c; then 
echo "---------"
./write_obj -p /dev/ttyS0 -w pinguim.gif ;
else
echo "COMPILATION ERROR";
fi
