#include "application.h"

static int currFrame=0;

int llopen(int number, int type){
    char port[48];
    sprintf(port, "/dev/ttyS%d", number);

    if (type == RECEIVER){
        return openReader(port);
    }
    else if(type == TRANSMITTER){
        return openWriter(port);
    }


    return -1;
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
    //Manda set (tem que receber UA)
    printf("Message: %s\n", buffer);

    infoFrame frame = messageStuffing(buffer, length);

    if(write(fd,frame.rawData,frame.rawSize)>=0)
        printf("Message sent\n");
    else
        printf("Message not sent\n");

    
    //stop and wait part

}





int llread(int fd, char* buffer){







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

    frame.rawData=(unsigned char*)malloc((frame.size+10)*sizeof(unsigned char));
    frame.rawData[0]=frame.flag;
    frame.rawData[1]=frame.address;
    frame.rawData[2]=frame.control;
    frame.rawDara[3]=frame.bcc1;

    for (int i=0; i< frame.size;i++){
        frame.rawData[i+4]=frame.data[i];
    }
    
    if(frame.bcc2==ESC){
        frame.rawData[frame.size+5]=frame.bcc2;
        frame.rawData[frame.size+6]=ESC_ESC;
        frame.rawData[frame.size+7]=frame.flag;
        frame.rawSize=frame.size+8;
    } else if(frame.bcc2==FLAG){
        frame.rawData[frame.size+5]=ESC;
        frame.rawData[frame.size+6]=ESC_FLAG;
        frame.rawData[frame.size+7]=frame.flag;
        frame.rawSize=frame.size+8;
    }
    else {
        frame.rawData[frame.size+5]=frame.bcc2;
        frame.rawData[frame.size+6]=frame.flag;
        frame.rawSize=frame.size+7;
    }


    return frame;
}


int messageDestuffing(unsigned char* rawData){
    infoFrame frame.rawData=rawData;




}



