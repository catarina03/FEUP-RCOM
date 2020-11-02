#include "macros.h"



struct linkLayer {
    char port[20];/*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate;/*Velocidade de transmissão*/
    unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;/*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso defalha*/
    char frame[MAX_SIZE];/*Trama*/
};

unsigned char readSupervisionFrame(int fd);

void sendSupervisionFrame(int fd, unsigned char control, unsigned char bcc);

unsigned char receiveSupervisionFrame(int fd, unsigned char control);

void sendMessage(int fd, unsigned char msg);

int receiveMessage(int fd, unsigned char msg);

void setAlarm();

int getAlarmFlag();

int getAlarmCounter();

void setAlarmFlag(int flag);


