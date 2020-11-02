#include "macros.h"
#include "protocol.h"


typedef struct{
    unsigned char flag;
    unsigned char control;
    unsigned char address;
    unsigned char bcc1;  //?
    unsigned char bcc2;  //?
    unsigned char * data; //after stuffing
    unsigned char * rawData;  //before stuffing
    int size;
    int rawSize;
}infoFrame;

typedef struct{
    unsigned char flag;
    unsigned char address;
    unsigned char control;
    unsigned char bcc;   
}supervisionFrame;

void buildSupervisionFrame(supervisionFrame *frame, unsigned char controlByte);

int llopen(char *port, int type);

int llwrite(int fd, char* buffer,int length);

int llread(int fd, char* buffer);

int llclose(int fd, int type);

infoFrame messageStuffing(char* buff, int length);

infoFrame messageDestuffing(char*buff);

int openReader(char *port);

int openWriter(char *port);

void printFrame(infoFrame frame);