#include "application.h"

static int currFrame=0;

static int fd;

static int alarmFlag = 1;

static int alarmCounter=0;

static int STOP=FALSE;

struct termios oldtio;


int openReader(char *port){

    //Builds supervision frame
    supervisionFrame reply;
    buildSupervisionFrame(&reply, UA);

    if(receiveSupervisionFrame(fd,SET) >= 0){
        sendSupervisionFrame(fd,reply.control, reply.bcc);
    }
    else{
        printf("Did not UA. exited program\n ");
        llclose(fd, RECEIVER);
        exit(-1);
    }

    printf("Sent SET frame and received UA successfully\n");
    return TRUE;  
}


int openWriter(char *port){

    //Builds supervision frame
    supervisionFrame start;
    buildSupervisionFrame(&start, SET);
    
    sendSupervisionFrame(fd, start.control, start.bcc);
    
    /*
    do{
        alarm(3);
        setAlarmFlag(0);
        while(!getAlarmFlag()){
            if(!receiveSupervisionFrame(fd, UA)){
                break;
            }
        }
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3 && STOP==FALSE);
    setAlarmCounter(0);
    */

    receiveSupervisionFrame(fd, UA);



    return TRUE;
}


int closeWriter(int fd){

    printf("Closing writer...\n");

    supervisionFrame close;
    buildSupervisionFrame(&close, DISC);

    sendSupervisionFrame(fd, close.control, close.bcc);

    printf("sent DISC frame\n");

    supervisionFrame closeUA;
    buildSupervisionFrame(&closeUA, UA);

    if (receiveSupervisionFrame(fd, DISC)){
        printf("received DISC frame\n");
        sendSupervisionFrame(fd, closeUA.control, closeUA.bcc);
    }


    return TRUE;
}


int closeReader(int fd){

    printf("Closing reader...\n");
    if (receiveSupervisionFrame(fd, DISC) < 0) return -1;

    //Builds supervision frame
    supervisionFrame close;
    buildSupervisionFrame(&close, DISC);

    sendSupervisionFrame(fd, close.control, close.bcc);

    if (receiveSupervisionFrame(fd, UA) < 0) return -1;


    return 1;
}


void buildSupervisionFrame(supervisionFrame *frame, unsigned char controlByte){
  frame->flag = FLAG;
  frame->address = A;
  frame->control = controlByte;
  frame->bcc = frame->address ^ frame->control;
}


int llopen(char *port, int type){
    struct termios newtio;

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {
        perror(port); 
        exit(-1); 
    }

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
    newtio.c_cc[VMIN]     = 1;   // blocking read until 5 chars received

    // VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    // leitura do(s) pr�ximo(s) caracter(es)

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    if (type == RECEIVER){
        return openReader(port);
    }
    else if(type == TRANSMITTER){
        return openWriter(port);
    }

    return TRUE;
}




int llclose(int fd, int type){


    if (type == RECEIVER){
        return closeReader(fd);
    }
    else if(type == TRANSMITTER){
        return closeWriter(fd);
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    return 0;
}





int llwrite(int fd, char* buffer,int length){

    //Init
    printf("Message: %s\n", buffer);

    infoFrame frame = messageStuffing(buffer, length);

    int size;
    if((size=write(fd, frame.rawData, frame.rawSize)>=0))
        printf("Message sent\n");
    else
        printf("Message not sent\n");

    
    //stop and wait part
    usleep(STOP_AND_WAIT);

    unsigned char response = readSupervisionFrame(fd);

    if(response==CONTROL_RJ(1)||response==CONTROL_RJ(0)){
        printf("Negative response\n");
        return -1;
    }
    else{
        return size;
    }
}





int llread(int fd, char* buffer){



    infoFrame frame = messageDestuffing(buffer);


    memcpy(buffer, frame.data,frame.size);
    unsigned char bcc2=0xff;
    for (int i=0;i<frame.size;i++){
        bcc2^=frame.data[i];
    }
    if(frame.bcc1!=(frame.address ^frame.control) || frame.bcc2!=bcc2){
        sendMessage(fd, CONTROL_RJ(currFrame));
        return -1;
    }
    else{
        sendMessage(fd, CONTROL_RR(currFrame));
        if(currFrame)
            currFrame--;
        else
            currFrame++;    
            
        return frame.size;
    }



}






infoFrame messageStuffing(char* buff, int length){
    infoFrame frame;

    frame.flag=FLAG;
    frame.address=A;
    frame.control=CONTROL_I(currFrame);
    frame.bcc1=frame.address ^ frame.control;
    frame.bcc2=0xff;
    int size=length;
    frame.data=(unsigned char*)malloc(size*sizeof(unsigned char));
    frame.size=0;
    for(int i =0; i<length;i++){
        if(buff[i]==ESC){
            frame.data=(unsigned char*)realloc(frame.data, ++size);
            frame.data[frame.size++]=ESC;
            frame.data[frame.size++]=ESC_ESC;
        }
        else if(buff[i]==FLAG){
            frame.data=(unsigned char*)realloc(frame.data, ++size);
            frame.data[frame.size++]=ESC;
            frame.data[frame.size++]=ESC_FLAG;
        }
        else
            frame.data[frame.size++]=buff[i];
        
        frame.bcc2=buff[i]^frame.bcc2;
    }

    frame.rawData=(unsigned char*)malloc((frame.size+8)*sizeof(unsigned char));
    frame.rawData[0]=frame.flag;
    frame.rawData[1]=frame.address;
    frame.rawData[2]=frame.control;
    frame.rawData[3]=frame.bcc1;

    for (int i=0; i< frame.size;i++){
        frame.rawData[i+4]=frame.data[i];
    }
    
    if(frame.bcc2==ESC){
        frame.rawData[frame.size+5]=frame.bcc2;
        frame.rawData[frame.size+6]=ESC_ESC;
        frame.rawData[frame.size+7]=frame.flag;
        frame.rawSize=frame.size+8;
    }
}

infoFrame messageDestuffing( char* buff){

    infoFrame frame;
    frame.rawData=(unsigned char*) malloc (sizeof(unsigned char));
    int flags = 0;
    int extra=0;
    unsigned char msg;
    int size=0;
    while(flags!=2){
        read(fd, &msg, 1);


        if (msg == FLAG && flags == 0) {
            flags = 1;
            continue;
        }
        else if (msg == FLAG && flags == 1) {
            flags = 2;
            break;
        }
        frame.rawData[size-extra] = msg;
        frame.rawData = (unsigned char*) realloc (frame.rawData, (size-extra+1));

        
        if (size>0 && frame.rawData[size-extra-1] == ESC) {
            
            if (frame.rawData[size-extra] == ESC_ESC){
                extra++;           
            }
            else if (frame.rawData[size-extra+1] == ESC_FLAG){
                frame.rawData[size-extra-1]=FLAG;
                extra++;
            }
            
        }
        size++;
    }
    size-=extra;
    frame.flag=FLAG;
    frame.address = frame.rawData[0];
    frame.control = frame.rawData[1];
    frame.bcc1 = frame.rawData[2];
    
    for (int i = 3; i < size - 1; i++) {
        frame.data[i-3] = frame.rawData[i];
        
    }
    frame.bcc2=frame.rawData[size-1];
    frame.size=size-4;
    return frame;
    
}



void printFrame(infoFrame frame){
    
    


}

