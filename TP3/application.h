#pragma once
#include "macros.h"
#include "supervision.h"
#include "alarme.h"
#include "parseNbuild.h"
#include "printers.h"
#include "frameStructs.h"





int llopen(char *port, int type);

int llclose(int fd, int type);



int openReader(int fd);

int receiverApp(int fd);

int llread(int fd, char* buffer);

infoFrame messageDestuffing(unsigned char*buff,int fd);

int closeReader(int fd);



int openWriter(int fd);

int transmitterApp(char *path,int fd);

int llwrite(int fd, unsigned char* buffer,int length);

infoFrame messageStuffing(unsigned char* buff, int length);

int closeWriter(int fd);
