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
  int data_field_size;      /**< @brief The size of the data array - [1..PACKET_SIZE] */
  unsigned char data[2*PACKET_SIZE]; /**< @brief The data array */

  unsigned char *raw_bytes; /**< @brief The array containing unprocessed bytes */
  int raw_bytes_size;       /**< @brief The size of the raw_bytes array */
} data_packet_t;

void buildSupervisionFrame(supervisionFrame *frame, unsigned char controlByte);

int llopen(char *port, int type);

int llwrite(int fd, char* buffer,int length);

int llread(int fd, char* buffer);

int llclose(int fd, int type);

infoFrame messageStuffing(char* buff, int length);

infoFrame messageDestuffing(char*buff,int fd);

int openReader(int fd);

int openWriter(int fd);

int closeReader(int fd);

int closeWriter(int fd);

unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize);

controlFrame parseControlFrame(unsigned char *rawBytes, int size);

int transmitterApp(char *path,int fd);

int receiverApp(int fd);

void printFrame(infoFrame frame);

void print_control_packet(controlFrame control);

data_packet_t parse_data_packet(unsigned char *raw_bytes, int size);

void print_data_packet(data_packet_t* packet, int full_info);