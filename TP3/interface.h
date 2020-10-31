#include "macros.h"
#include "application.h"

typedef struct{
    char *port;  //Path da porta de série passado pelo user
    int fileDescriptor;/*Descritor correspondente à porta série*/
    int status;/*TRANSMITTER | RECEIVER*/
}applicationLayer;