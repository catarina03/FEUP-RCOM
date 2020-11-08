#pragma once
#include "macros.h"



typedef struct {
    unsigned char flag;
    unsigned char control;
    unsigned char address;
    unsigned char bcc1;  
    unsigned char bcc2;  
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
}controlFrame;


typedef struct {
  unsigned char control;    /**< @brief The control byte - [DATA] */
  unsigned char sequence;   /**< @brief The sequence byte - index on global data */
  int dataSize;      /**< @brief The size of the data array - [1..PACKET_SIZE] */
  unsigned char data[2*MAX_SIZE]; /**< @brief The data array */

  unsigned char *rawBytes; /**< @brief The array containing unprocessed bytes */
  int rawSize;       /**< @brief The size of the raw_bytes array */
}dataFrame;


