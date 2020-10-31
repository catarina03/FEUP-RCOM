#include "macros.h"
#include "protocol.h"
#include "application.h"

typedef struct{
    char *fileDescriptor;/*Descritor correspondente à porta série*/
    int status;/*TRANSMITTER | RECEIVER*/
}applicationLayer;