#!/bin/bash

#rm download

if gcc -o download -Wall download.c connection.c; then
./download ftp://rcom:rcom@netlab1.fe.up.pt/pipe.txt;
else
echo "COMPILATION ERROR";
fi

# ./download ftp://netlab1.fe.up.pt/pub.txt;
# ./download ftp://ftp.up.pt/pub/gnu/GNUinfo/Audio/index.txt;
# ./download ftp://rcom:rcom@netlab1.fe.up.pt/pipe.txt;
# ./download ftp://rcom:rcom@netlab1.fe.up.pt/files/pic1.jpg;
# ./download ftp://rcom:rcom@netlab1.fe.up.pt/files/pic2.png;
# ./download ftp://rcom:rcom@netlab1.fe.up.pt/files/crab.mp4; <-best one but heavy
# ./download ftp://ftp.up.pt/pub/deb-multimedia/CSS/style.css;

# ./download "ftp://[<user>:<password>@]<host>/<url-path>" 
# ./download "ftp://<host>/<url-path>" 

