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
  int filesizeSize; 
  int filenameSize;

  unsigned char *rawBytes;   
  int rawSize;         
} controlFrame;

typedef struct{
    unsigned char flag;
    unsigned char address;
    unsigned char control;
    unsigned char bcc;
}supervisionFrame;


typedef struct {
  unsigned char control;    /**< @brief The control byte - [DATA] */
  unsigned char sequence;   /**< @brief The sequence byte - index on global data */
  int dataSize;      /**< @brief The size of the data array - [1..PACKET_SIZE] */
  unsigned char data[2*PACKET_SIZE]; /**< @brief The data array */

  unsigned char *rawBytes; /**< @brief The array containing unprocessed bytes */
  int rawSize;       /**< @brief The size of the raw_bytes array */
} dataFrame;


int llopen(char *port, int type);

int llwrite(int fd, unsigned char* buffer,int length);

int llread(int fd, char* buffer);

int llclose(int fd, int type);

infoFrame messageStuffing(unsigned char* buff, int length);

infoFrame messageDestuffing(unsigned char*buff,int fd);

int openReader(int fd);

int openWriter(int fd);

int closeReader(int fd);

int closeWriter(int fd);

unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize);

controlFrame parseControlFrame(unsigned char *rawBytes, int size);

int transmitterApp(char *path,int fd);

int receiverApp(int fd);

void printInfoFrame(infoFrame frame);

void printControlFrame(controlFrame frame);

dataFrame parseDataFrame(unsigned char *rawBytes, int size);

void printDataFrame(dataFrame frame, int full_info);