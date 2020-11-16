#include "parseNbuild.h"



unsigned char *buildControlFrame(char ctrlField, unsigned long fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize) {
    unsigned char *frame=(unsigned char*) malloc(sizeof(unsigned char)*frameSize);
    printf(" ----Frame Size -------%d\n",frameSize);
    frame[0] = ctrlField;
    frame[1] = FILE_SIZE;
    frame[2] = L1;
    memcpy(&frame[3], &fileSize, L1);
    /*for(int i=0; i<L1;i++)
        printf("j = 0x%02x\n",frame[3+i]);*/
    frame[3+L1] = FILE_NAME;
    frame[4+L1] = L2;
    memcpy(&frame[5+L1], fileName, L2);
    printf(" -----File Size -------%d\n",fileSize);
    printf(" -----Size Size -------%d\n",L1);
    printf(" -----File Name -------%s\n",fileName);
    printf(" -----Name Size -------%d\n",L2);

    return frame;
}



dataFrame parseDataFrame(unsigned char *rawBytes, int size) {
    dataFrame frame;
    memset(&frame, 0, sizeof( dataFrame));
    frame.rawBytes = rawBytes;
    frame.rawSize = size;
    frame.control = rawBytes[0];
    frame.sequence = rawBytes[1];

    frame.dataSize = (rawBytes[2] << 8) | rawBytes[3];
    for (int i = 0; i < frame.dataSize; i++) {
        frame.data[i] = rawBytes[i+4];
    }

    return frame;
}



controlFrame parseControlFrame(unsigned char *rawBytes, int size) {
    controlFrame frame;
    memset(&frame, 0, sizeof(controlFrame));
    frame.control = rawBytes[0];

    frame.filenameSize = 0;

    frame.filesizeSize = 0;
    int len;
    int fileSizeFlag=1;
    for (int i = 1; i < size;i++) {
        if (rawBytes[i] == FILE_SIZE && fileSizeFlag) {
            fileSizeFlag=0;
            // printf("Parsing file size\n");
        len = rawBytes[++i];
        //printf("len %d\n",len);
        frame.fileSize = (unsigned char*) malloc(len*sizeof(unsigned char));

        for (int j = 0; j <len;j++) {
            frame.fileSize[j] = rawBytes[++i];
            //printf("j %d - byte 0x%02x\n",j,frame.fileSize[j]);
            
        }
        frame.filesizeSize=len;
        
        }
        else if (rawBytes[i] == FILE_NAME) {
            //printf("Parsing file name\n ");
        len = rawBytes[++i];
        //printf("len %d\n",len);
        frame.fileName = (unsigned char *) malloc ((len+1)*sizeof(unsigned char));
        
        for (int j = 0; j < len;j++) {
            frame.fileName[j] = rawBytes[++i];
            //printf("j %d - byte 0x%02x\n",j,frame.fileName[j]);
        }
        frame.filenameSize=len;
        }
        
    }
    /*for (int i=1;i<size;i++)
        printf("RawData byte %d- 0x%02x\n",i,rawBytes[i]);*/

    frame.rawSize=size;
    frame.fileName[frame.filenameSize] = '\0';
    
    frame.rawBytes=rawBytes;
    
    
    
    return frame;
}
