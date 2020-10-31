#include "protocol.h"

void sigalarm_handler(int signo){
  
  alarmFlag=0;
  alarmCounter++;

}

void setAlarm(){
  struct sigaction act_alarm;
        act_alarm.sa_handler = sigalrm_handler;
        sigemptyset(&act_alarm.sa_mask);
        act_alarm.sa_flags = 0;
        
        if (sigaction(SIGALRM,&act_alarm,NULL) < 0)  {        
            fprintf(stderr,"Unable to install SIGALARM handler\n");        
            exit(1);  
        } 
}

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





void sendMessage(int fd, unsigned char msg) {
  unsigned char mesh[5];
  mesh[0]=FLAG;
  mesh[1]=A;
  mesh[2]=msg;
  mesh[3]=mesh[1]^mesh[2];
  mesh[4]=FLAG;
  write(fd,mesh,5);
}



int receiveMessage(int fd, unsigned char msg) {
  int part=0;
  unsigned char rcv_msg;
  printf("Reading...\n");
  while (part!=5) {

    read(fd, &rcv_msg,1);
    printf("byte: %d\n", rcv_msg);
    switch (part) {
      case 0:
        if(rcv_msg==FLAG){
          part=1;
          printf("FLAG: %c\n",rcv_msg);
        }
        break;
      case 1:
        if(rcv_msg==A){
          part=2;
          printf("A: %c\n",rcv_msg);
        }
        else {
          if(rcv_msg==FLAG)
            part=1;
          else
            part=0;
        }
        break;
      case 2:
        if(rcv_msg==msg){
          part=3;
          printf("Control: %c\n",rcv_msg);
        }
        else
          part=0;
        break;
      case 3:
        if(rcv_msg==(A^msg)){
          part=4;
          printf("Control BCC: %c\n",rcv_msg);
        }
        else
          part=0;
        break;
      case 4:
        if(rcv_msg==FLAG) {
          part = 5;
          printf("FINAL FLAG: %c\nReceived Control\n",rcv_msg);
        }
        else
          part=0;
        break;
      default:
        break;
    }
  }
  
  return TRUE;
}

