#include "application.h"

static int currFrame=0;

static int fd;

static int alarmFlag = 1;

static int alarmCounter=0;

static int STOP=FALSE;


int openReader(char *port){

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(port); exit(-1); }

    if(receiveMessage(fd,SET)){
        printf("Receiving SET\n");
        resendMessage(fd,UA); //Recieving 
    }

    



    return 0;
}

int openWriter(char *port){

    //Builds supervision frame
    supervisionFrame start;
    buildProtectionFrame(&start, SET);

    fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(port); exit(-1); }

    //Sending supervision frame
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



    return 0;
}


void buildProtectionFrame(supervisionFrame *frame, unsigned char controlByte){
  frame->flag = FLAG;
  frame->address = A;
  frame->control = controlByte;
  frame->bcc = frame->address ^ frame->control;
}


int llopen(char *port, int type){

    if (type == RECEIVER){
        return openReader(port);
    }
    else if(type == TRANSMITTER){
        return openWriter(port);
    }

    return 0;
}




int llclose(int fd, int type){

    if (type == RECEIVER){
        return closeReader(fd);
    }
    else if(type == TRANSMITTER){
        return closeWriter(fd);
    }


    return -1;
}





int llwrite(int fd, char* buffer,int length){

    //Init

    int size;    //Manda set (tem que receber UA)
    prisize=(ntf("Message: %s\n", buffer);
)
    infoFrame frame = messageStuffing(buffer, length);

    int size;
    if(size==(write(fd,frame.rawData,frame.rawSize)>=0))
        printf("Message sent\n");
    else
        printf("Message not sent\n");

    
    //stop and wait part
    usleep(STOP_AND_WAIT);

    unsigned char response =readSupervisionFrame(int fd);

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


    memcpy(buff, frame.data,frame.size);
    unsigned char bcc2=0xff;
    for (int i=0;i<frame.size;i++){
        bcc2^=frame.data[i];
    }
    if(frame.bcc1!=(frame.address ^frame.control) || frame.bcc2!=bcc2){
        //enviar resposta negativa
        return -1;
    }
    else{
        //enviar resposta positiva
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
            
            if (frame.rawData[size-extra] == ESC_ESC)
                extra++;           }
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

