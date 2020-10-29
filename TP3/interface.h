#include "macros.h"

typedef struct{
    int fileDescriptor;/*Descritor correspondente à porta série*/
    int status;/*TRANSMITTER | RECEIVER*/
}applicationLayer;