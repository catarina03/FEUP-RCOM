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

typedef struct {
  unsigned char control;      
  unsigned char *fileSize;   
  unsigned char *fileName;   
  unsigned int filesizeSize; 

  unsigned char *rawBytes;   
  int rawSize;         
} controlFrame;

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

int closeReader(int fd);

int closeWriter(int fd);

unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize);

unsigned char *parseControlFrame(unsigned char *raw_bytes, int size);

int transmitterApp(char *path);

int recieverApp(char *path);

void printFrame(infoFrame frame);