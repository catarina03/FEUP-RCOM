/*Non-Canonical Input Processing*/

//#include "macros.h"
#include "application.h"


/*

static int STOP=FALSE;

static int alarmFlag = 1;

static int alarmCounter=0;

static int fd;


void sendSupervisionFrame(int fd, supervisionFrame *frame) {
  unsigned char msg[5];
  msg[0] = frame->flag;
  msg[1] = frame->address;
  msg[2] = frame->control;
  msg[3] = frame->bcc;
  msg[4] = frame->flag;
  if (write(fd,msg,5) == -1) {
    perror("Supervision frame write failure!\n");
  }
}



void receiveSupervisionFrame(int *part, unsigned char *msg, unsigned char *control) {
  printf("Part: %d\n", *part);
  printf("msg: %c\n", *msg);
  switch (*part)
  {
  case 0:
    if(*msg==FLAG){
      *part=1;
      printf("FLAG: %c\n",*msg);
    }
         
    break;
  case 1:
    if(*msg==A){
      *part=2;
      printf("A: %c\n",*msg);
    }
    else {
      if(*msg==FLAG)
        *part=1;
      else
        *part=0;
    }
    break;
  case 2:
    if(*msg==*control){
      *part=3;
      printf("UA: %c\n",*msg);
    }
    else
      *part=0;
    break;
  case 3:
    if(*msg==(UA_BCC)){
      *part=4;
      printf("UA_BCC: %c\n",*msg);
    }
    else
      *part=0;
    break;
  case 4:
    if(*msg==FLAG) {
      STOP = TRUE;
      printf("FINAL FLAG: %c\nReceived UA\n",*msg);
    }
    else
      *part=0;
    break;
  default:
    break;
  }
}

*/





/*void receiveResponse(int *part, unsigned char msg) {
  
      if(*part==0 && msg==FLAG){
        *part++;
        printf("FLAG: %c\n",msg);
      }
      else if(*part==1){ 
        if(msg==A){
          *part++;
          printf("A: %c\n",msg);
        }
        else 
            *part=0;
      }
      else if(*part==2){ 
        if(msg==UA){
          *part++;
          printf("C: %c\n",msg);
        }
        else
          *part=0;
      }
      else if(*part==3){ 
        if(msg==(UA_BCC)){
          *part++;
          printf("UA_BCC: %c\n",msg);
        }
        else
          *part=0;
      }
      else if(*part==4){ 
        if(msg==FLAG) {
          STOP = TRUE;
          printf("FINAL FLAG: %c\nReceived UA\n",msg);
        }
        else
          *part=0;
      }
    
}*/


/*

int main(int argc, char** argv)
{

    struct sigaction act_alarm;
    act_alarm.sa_handler = sigalrm_handler;
    sigemptyset(&act_alarm.sa_mask);
    act_alarm.sa_flags = 0;
    
    if (sigaction(SIGALRM,&act_alarm,NULL) < 0)  {        
        fprintf(stderr,"Unable to install SIGALARM handler\n");        
        exit(1);  
    }   

    //int c;
    struct termios oldtio,newtio;
    //int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


    //Open serial port device for reading and writing and not as controlling tty
    //because we don't want to get killed if linenoise sends CTRL-C.


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { //save current port settings
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    //set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   // inter-character timer unused 
    newtio.c_cc[VMIN]     = 5;   // blocking read until 5 chars received



   // VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
   // leitura do(s) pr�ximo(s) caracter(es)



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //printf("New termios structure set\n");

    // O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    // o indicado no gui�o 

    sendMessage(fd, SET);
    alarm(3);  
    int times=0;
    while(alarmCounter < 3 && !STOP){
      unsigned char replybuffer;
      while (!STOP && alarmFlag){
        if(read(fd, &replybuffer, 1) >=0){
          receiveResponse(&times,&replybuffer);
        }          
      }
      if(!STOP){
        printf("Timed-out\n");
      }
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;
}


*/