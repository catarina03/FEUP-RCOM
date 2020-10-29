#include "macros.h"


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


int llopen(int number, int type);

int llwrite(int fd, char* buffer,int length);

int llread(int fd, char* buffer);

int llclose(int fd, int type);

infoFrame messageStuffing(char* buff, int length);