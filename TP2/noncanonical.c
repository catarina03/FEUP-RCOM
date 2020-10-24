/*Non-Canonical Input Processing*/

#include "macros.h"


static int STOP=FALSE;







void receiveMessage(int *part, unsigned char *msg) {
  
      if(*part==0 && *msg==FLAG){
        *part++;
        printf("FLAG: %c\n",*msg);
      }
      else if(*part==1){ 
        if(*msg==A){
          *part++;
          printf("A: %c\n",*msg);
        }
        else 
            *part=0;
      }
      else if(*part==2){ 
        if(*msg==SET){
          *part=3;
          printf("C: %c\n",*msg);
        }
        else
          *part=0;
      }
      else if(*part==3){ 
        if(*msg==(SET_BCC)){
          *part++;
          printf("UA_BCC: %c\n",*msg);
        }
        else
          *part=0;
      }
      else if(*part==3){ 
        if(*msg==FLAG) {
          STOP = TRUE;
          printf("FINAL FLAG: %c\nReceived UA\n",*msg);
        }
        else
          *part=0;
      }
    
}






int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //printf("New termios structure set\n");


    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,255);   /* returns after 5 chars have been input */
      buf[res]=0;     
                /* so we can printf... */
      printf(":%s:%d\n", buf, res);
      if (buf[res-1]=='\n') STOP=TRUE;
    }

    printf("content of buf: %s", buf);

    int res1=0;
    res1 = write(fd,buf,strlen(buf));   
    if (res!=res1){
      printf("Warning: Diferent number of bytes read and sent\n");
    }
    printf("%d bytes written\n", res1);

	



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}