#!/bin/bash

# rm write_obj

#clear

if gcc -o write_obj -Wall interface.c application.c writenoncanonical.c noncanonical.c alarme.c protocol.c; then 
echo "---------"
./write_obj -p /dev/ttyS10 -w pinguim.gif ;
else
echo "COMPILATION ERROR";
fi
