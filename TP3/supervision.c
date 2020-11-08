#include "supervision.h"



int receiveSupervisionFrame(int fd, unsigned char control) {
  int times=0;
  unsigned char msg;
  //printf("Reading response...\n");
  while (times!=5) {
    read(fd,&msg,1);
    if(times==0){
        if(msg==FLAG){
          times++;
          //printf("FLAG- 0x%02x\n",msg);
        }
    }
    else if(times==1){
        if(msg==A){
          times++;
          //printf("A- 0x%02x\n",msg);
        }
        else {
          if(msg==FLAG)
            times--;
          else
            times=0;
        }
    }
    else if (times==2){
        if(msg==control){
          times++;
          //printf("C- 0x%02x\n",msg);
          control = msg;
        }
        else
          times=0;
    }
    else if(times==3){
        if(msg==(A^control)){
          times++;
          //printf("BCC- 0x%02x\n",msg);
        }
        else
          times=0;
    }
    else if(times==4){
        if(msg==FLAG) {
          times++;
          //printf("SECOND FLAG- 0x%02x\n",msg);
          return TRUE;
        }
        else
          times=0;
    }
  }
  return FALSE;
}

unsigned char readSupervisionFrame(int fd){
  int times=0;
  unsigned char msg, control;
  //printf("Reading response...\n");
  while (times!=5) {
    read(fd,&msg,1);
    if(times==0){
        if(msg==FLAG){
          times++;
          //printf("FLAG- 0x%02x\n",msg);
        }
    }
    else if(times==1){
        if(msg==A){
          times++;
          //printf("A- 0x%02x\n",msg);
        }
        else {
          if(msg==FLAG)
            times--;
          else
            times=0;
        }
    }
    else if (times==2){
        if(msg==CONTROL_RJ(1) || msg==CONTROL_RJ(0)
        || msg==CONTROL_RR(1) || msg==CONTROL_RR(0)){
          times++;
          //printf("C- 0x%02x\n",msg);
          control = msg;
        }
        else
          times=0;
    }
    else if(times==3){
        if(msg==(A^control)){
          times++;
          //printf("BCC- 0x%02x\n",msg);
        }
        else
          times=0;
    }
    else if(times==4){
        if(msg==FLAG) {
          times++;
          //printf("SECOND FLAG- 0x%02x\n",msg);
        }
        else
          times=0;
    }
  }
  return control;
}


void sendSupervisionFrame(int fd, unsigned char msg) {
  unsigned char mesh[5];
  mesh[0]=FLAG;
  mesh[1]=A;
  mesh[2]=msg;
  mesh[3]=mesh[1]^mesh[2];
  mesh[4]=FLAG;
  write(fd,mesh,5);
}




