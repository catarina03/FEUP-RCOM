#pragma once
#include "macros.h"
#include "frameStructs.h"


unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize);

controlFrame parseControlFrame(unsigned char *rawBytes, int size);

dataFrame parseDataFrame(unsigned char *rawBytes, int size);