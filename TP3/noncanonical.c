/*Non-Canonical Input Processing*/

#include "macros.h"

static int STOP=FALSE;




/*
int recieveMessage( unsigned char msg_type) {
    printf("Reading...\n");
    int part = 0;
    unsigned char msg[5];
    while (part!=5) {
      printf("%c", part);

      read(fd, &msg[part], 1);
      if(part==0 && msg[part]==FLAG){
        printf("FLAG: %c\n",msg[part]);
        part++;
        
      }
      else if(part==1){ 
        if(msg[part]==A){
          printf("A: %c\n",msg[part]);
          part++;
          
        }
        else 
            part=0;
      }
      else if(part==2){ 
        if(msg[part]==msg_type){  //SET or UA
          printf("C: %c\n",msg[part]);
          part=3;
        }
        else
          part=0;
      }
      else if(part==3){ 
        if(msg[part]==(SET_BCC)){
          printf("SET_BCC: %c\n",msg[part]);
          part++;
        }
        else
          part=0;
      }
      else if(part==4){ 
        if(msg[part]==FLAG) {
          STOP = TRUE;
          printf("FINAL FLAG: %c\nReceived UA\n",msg[part]);
          return TRUE;
        }
        else
          part=0;
      }
    }
    if (part == 5) return TRUE;
    else return FALSE;
}*/



/*



int main(int argc, char** argv)
{
    //int fd,c; 
    int res;
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

    // set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   // inter-character timer unused
    newtio.c_cc[VMIN]     = 5;   // blocking read until 5 chars received 



  
    //VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    //leitura do(s) pr�ximo(s) caracter(es)
  



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //printf("New termios structure set\n");


    //O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 


  unsigned char buff;
  if(receiveMessage(fd,SET))
    resendMessage(fd,UA); //Recieving 


  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return 0;
}


*/
