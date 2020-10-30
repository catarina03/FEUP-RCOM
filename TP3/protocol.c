#include "protocol.h"


unsigned char readSupervisionFrame(int fd){
  int times=0;
  unsigned char msg, control;
  printf("Reading response...\n");
  while (times!=5) {
    read(fd,&msg,1);
    if(times==0){
        if(msg==FLAG){
          times++;
          printf("FLAG- 0x%c\n",msg);
        }
    }
    else if(times==1){
        if(msg==A){
          times++;
          printf("A- 0x%c\n",msg);
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
        || msg==CONTROL_RR(1) || msg==CONTROL_RR(1)){
          times++;
          printf("C- 0x%c\n",msg);
          control = msg;
        }
        else
          times=0;
    }
    else if(times==3){
        if(msg==(A^control)){
          times++;
          printf("BCC- 0x%c\n",msg);
        }
        else
          times=0;
    }
    else if(times==4){
        if(msg==FLAG) {
          times++;
          printf("SECOND FLAG- 0x%c\n",msg);
        }
        else
          times=0;
    }
  }
  return control;
}