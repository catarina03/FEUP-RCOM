#pragma once
#include "macros.h"
#include "alarme.h"



unsigned char readSupervisionFrame(int fd);

int receiveSupervisionFrame(int fd, unsigned char control);

int sendSupervisionFrame(int fd, unsigned char msg);




