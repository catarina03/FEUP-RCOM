#!/bin/bash

rm write_obj

#clear

if gcc -o write_obj -Wall interface.c application.c protocol.c; then 
echo "---------"
valgrind --leak-check=yes ./write_obj -p /dev/ttyS10 -w pinguim.gif ;
else
echo "COMPILATION ERROR";
fi
