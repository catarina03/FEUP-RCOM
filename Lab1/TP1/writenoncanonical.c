/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>












#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



static int STOP=FALSE;

static int alarmFlag = 1;

static int alarmCounter=0;

static int fd;
static char buf[255];
static int res;


void resendMsg(){

    res = write(fd,buf,strlen(buf));   
    printf("%d bytes written\n", res);


}
void sigalrm_handler(int signo){
  
  alarmFlag=0;
  alarmCounter++;
  resendMsg();
	if(alarmCounter<3)
    alarm(3);

}




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


  
    int c;
    struct termios oldtio,newtio;
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    //gets(stdin);


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


/*
    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }

    */
    
    /*testing*/ /*
    buf[25] = '\n';
    
    res = write(fd,buf,255);   
    printf("%d bytes written\n", res);
    */

 

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

    
    int flag = 0;
    printf("Message: ");
    fgets(buf,255,stdin);
    resendMsg();
    alarm(3);  
    while(alarmCounter < 3 && !flag){
      char replybuffer[255];
      while (STOP==FALSE && alarmFlag){
          int res1;
          res1 = read(fd, replybuffer, 255);
          if(res1>=0){
            replybuffer[res1]=0;
            if (replybuffer[res1-1] == '\n') STOP=TRUE;
          }
      }
      if(STOP){
        printf("Message received: %s\n ",replybuffer);
        flag=1;
        break;
      }
      else{
        alarmFlag=1;
      }
      
    }








   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    return 0;


    



}
