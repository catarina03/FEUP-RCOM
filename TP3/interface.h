#include "macros.h"

typedef struct{
    char *fileDescriptor;/*Descritor correspondente à porta série*/
    int status;/*TRANSMITTER | RECEIVER*/
}applicationLayer;