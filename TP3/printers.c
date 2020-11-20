#include "printers.h"




void printDataFrame( dataFrame frame) {
  printf("\n     DATA     \n");
  printf("Control: -0x%02x\n", frame.control);
  printf("Data size: %d -0x%02x\n", frame.dataSize,
         frame.dataSize);
  printf("Sequence: %d -0x%02x\n", frame.sequence, frame.sequence);

  if (PRINT_ALL) {
    for (int i = 0; i < frame.dataSize; i++) {
      printf("DATA[%d]: %x -0x%02x\n", i, frame.data[i], frame.data[i]);
    }
  }
}


void printControlFrame( controlFrame frame){
    printf("\n       CONTROL      \n");
    printf("Control: %x\n", frame.control);
    printf("File size: %x\n", frame.fileSize);
    printf("File name: %s\n", frame.fileName);
    printf("Filesize size: %d\n", frame.filesizeSize);
    printf("Filename size: %d\n", frame.filenameSize);
    printf("Raw bytes: %x\n", frame.rawBytes);
    printf("Raw size: %d\n", frame.rawSize);
    
}

void printInfoFrame( infoFrame frame){
    printf("\n     Info     \n");
    printf("FLAG- 0x%x\n",frame.flag);
    printf("Address - 0x%x\n",frame.address);
    printf("Control - 0x%x\n",frame.control);
    printf("BCC1 - 0x%x",frame.bcc1);
    if(PRINT_ALL){
        for(int i =0;i<frame.size;i++){
            if(i%8==0){
                printf("\nData - ");
            }
            printf(" 0x%x,",frame.data[i]);
        }
    }
    printf("\nData Size - %d",frame.size);
    printf("\nBCC2 - 0x%x\n",frame.bcc2);
    printf("Second FLAG- 0x%x",frame.flag);
    if(PRINT_ALL){
        for(int i =0;i<frame.rawSize;i++){
            if(i%15==0){
                printf("\nRaw Data - ");
            }
            printf(" 0x%x,",frame.rawData[i]);
        }
    }
    printf("\nRaw Size - %d\n",frame.rawSize);
}

