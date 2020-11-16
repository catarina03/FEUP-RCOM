#include "application.h"


static int currFrame=0;

struct termios oldtio;



int llopen(char *port, int type){
    struct termios newtio;

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(port, O_RDWR | O_NOCTTY );
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

    setAlarm();

    if (type == RECEIVER){
        openReader(fd);
    }
    else if(type == TRANSMITTER){
        openWriter(fd);
    }

    return fd;
}




int llclose(int fd, int type){


    if (type == RECEIVER){
        closeReader(fd);
    }
    else if(type == TRANSMITTER){
        closeWriter(fd);
    }

    if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    return 0;
}








/* Start of Reader App */



int openReader( int fd){



    if(receiveSupervisionFrame(fd,SET) >= 0){
        sendSupervisionFrame(fd,UA);
    }
    else{
        printf("Did not UA. exited program\n ");
        llclose(fd, RECEIVER);
        exit(-1);
    }

    //printf("Reader received SET frame and sent UA successfully\n");
    return 0;  
}



int receiverApp(int fd){

    char buff[2*MAX_SIZE+7];
    int size;

    int state = 0;
    
    unsigned char* fileName;

    unsigned long fileSize=0;
    //START Control Frame
    while (!state) {
        memset(buff, 0, sizeof(buff));
        while ((size = llread(fd, buff)) < 0) {
            printf("Error reading\n");
            llclose(fd, RECEIVER);
            return -1;
        }
        controlFrame frame= parseControlFrame(buff,size);

        for(int i=0; i<frame.filesizeSize;i++)
            fileSize|=frame.fileSize[i]<<(8*i);
        
        printf("--------FILE SIZE-------%d\n",fileSize);
        fileName = frame.fileName;

        //printf("File Name: %s\n", fileName);

        printControlFrame(frame);
        if (frame.control == START_FRAME) 
            state = 1;
        
        free(frame.fileSize);
        free(frame.fileName);
        
    }


    // * DATA Frames
    unsigned char *fullMessage = (unsigned char*) malloc (fileSize*sizeof(unsigned char));
    //unsigned char fullMessage[fileSize];
    int index = 0;
    int currSequence = -1;
	int totalSize=0;
    while (state == 1) {
        memset(buff, 0, sizeof(buff));
        //printf("done memset\n");
        printf("Sequence: %d\n",currSequence);
        while ((size = llread(fd, buff)) <0) {
			memset(buff, 0, sizeof(buff));
            printf("Error reading\n");
        }
        
       // printf("exited llread\n");
        if (buff[0] == END_FRAME) {
            //printf("Received End Frame\n");
            state = 2;
            break;
        }
        dataFrame data = parseDataFrame(buff, size);
        totalSize+=data.dataSize;
        if (data.control != DATA) {
            continue;
        }
        
        //printDataFrame(data);
        for (int i =0;i<data.dataSize;i++){
            fullMessage[index+i] = data.data[i];
        }
        // * caso o numero de sequencia seja diferente do anterior deve atualizar o index
        if (currSequence != data.sequence) {
            currSequence = data.sequence;
            index += data.dataSize;
        }
    }
    //printf("--------TOTAL SIZE------%d\n",totalSize);
    //printf("--------FILE SIZE------%d\n",fileSize);


    // * END Control Frame
    if (state == 2) {
         controlFrame frame = parseControlFrame(buff, size);
        printControlFrame(frame);

        //char* name = (char*) malloc ((strlen(fileName) +7) * sizeof(char));
        char* name = (char*) malloc ((frame.filenameSize +7) * sizeof(char));
        //char name[frame.filenameSize +7];

        sprintf(name, "cloned_%s", frame.fileName); 
        
        FILE *fl = fopen(name, "wb");
        if (fl != NULL) {
            fwrite(fullMessage, sizeof (unsigned char), fileSize, fl);
            fclose(fl);
        }
        //printf("Received file\n");
        free(frame.fileSize);
        //printf("Freed file size\n");
        free(frame.fileName);
        //printf("Freed file name\n");
        free(name);
        //printf("Freed name\n");
        //free(fl);
    }

    // resets and closes the receiver fd for the port

    free(fullMessage);
    //printf("Freed everything\n");
    return 0;
}


int llread(int fd, char* buffer){
	unsigned char bcc2=0xff;
    infoFrame frame = messageDestuffing(buffer,fd,&bcc2);
    printInfoFrame(frame);
    //printf("Exited destuffing\n");
    
    
    //printf("bcc1: %x - %x address ^ control\n",frame.bcc1,frame.address ^frame.control);
    if(frame.bcc1!=(frame.address ^frame.control) || frame.bcc2 !=bcc2){
        sendSupervisionFrame(fd, CONTROL_RJ(currFrame));
        for (int i= 0; i<frame.rawSize;i++){
			if(i%10==0)
				printf("\n");
			printf("RD%d:%02x ",i,frame.rawData[i]);
		}
        printf("Sent Negative Response\n");
        free(frame.rawData);
        free(frame.data);
        return -1;
    }
    else{
		memcpy(buffer, frame.data,frame.size);
        sendSupervisionFrame(fd, CONTROL_RR(currFrame));
        //printf("Sent Positive Response\n");
        if(currFrame)
            currFrame--;
        else
            currFrame++;    
            
        free(frame.rawData);
        free(frame.data);
        return frame.size;
    }
}


 infoFrame messageDestuffing(unsigned char* buff,int fd ,unsigned char *bcc2){

    infoFrame frame;
    frame.rawData=(unsigned char*) malloc (sizeof(unsigned char)*(MAX_SIZE*2+7));
    frame.data = (unsigned char*) malloc(sizeof(unsigned char)*(MAX_SIZE*2));
    int flags = 0;
    unsigned char msg;
    int size=0;
    int lastEsc=0;
    while(flags!=2){
        //printf("read --");
        read(fd, &msg, 1);

        
        if (msg == FLAG && flags == 0) {
            flags = 1;
            //printf("Flag 1\n");
            continue;
        }
        else if (msg == FLAG && flags == 1) {
            flags = 2;
            //printf("Flag 2 - size %d\n",size);
            if (size>1 && frame.rawData[size-2] == ESC) {
				if (frame.rawData[size-1] == ESC_FLAG)
					frame.rawData[--size-1]=FLAG;
				else if (frame.rawData[size-1] == ESC_ESC)
                  size--;
			}
            break;
        }
        //printf("msg- 0x%02x size- %d rawData- 0x%02x",msg,size,frame.rawData);
        frame.rawData[size] = msg;
        
        //printf(" --r");
        if (size>0 && frame.rawData[size-1] == ESC && !lastEsc) {
            if (frame.rawData[size] == ESC_FLAG){
                frame.rawData[--size]=FLAG;
                lastEsc=1;
			}
            else if (frame.rawData[size] == ESC_ESC){
                  size--;
                  lastEsc=1;
			  }
							
        }
        else
         lastEsc=0;
        
        /*if(size>3){
			frame.data[size-3] = frame.rawData[size];
			//*bcc2^=frame.data[size-4];
		}
		else if(size==3)
			frame.data[size-3] = frame.rawData[size];*/

        size++;
        //printf("--new size-%d\n",size);
    }
    frame.rawData = (unsigned char*) realloc (frame.rawData, (size));
    frame.data = (unsigned char*) realloc (frame.data, (size-4));
    //frame.rawData = (unsigned char*) realloc (frame.rawData, (size));
    //printf("exits\n");
    frame.flag=FLAG;
    frame.address = frame.rawData[0];
    frame.control = frame.rawData[1];
    frame.bcc1 = frame.rawData[2];
    
    //printf("------ RAW SIZE------%d\n",size);
    for (int i = 0; i < size - 4; i++) {
		frame.data[i] = frame.rawData[i+3];
        *bcc2^=frame.data[i];
    }
    
    
    frame.bcc2=frame.rawData[size-1];
    frame.size=size-4;
    frame.rawSize=size;
    return frame;
    
}


int closeReader(int fd){

    printf("Closing reader...\n");
    

    
    do{
		alarm(ALARM_TIME);
        setAlarmFlag(0);
		int resp;
		while((resp=receiveSupervisionFrame(fd, DISC)) <-0){
			printf("Error recieving DISC Frame...\n"); 
		}
		
		
        if(sendSupervisionFrame(fd,DISC)!=5){
			printf("Error Sending DISC\n");
			continue;
		}
        
        printf("%d\n",getAlarmCounter());
        
        if(receiveSupervisionFrame(fd, UA)==TRUE){
            resetAlarm();
            printf("\nClosed Reader\n");
            return 0;
        }
        
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3);
    resetAlarm();

    
    printf("Error recieving UA Frame...\n");   
    return -1;
    
    
    
}


/* End of Reader App*/


/* Start of Writer App*/


int openWriter(int fd){


    
    
    do{
        sendSupervisionFrame(fd,SET);
        alarm(ALARM_TIME);
        setAlarmFlag(0);
        //printf("%d\n",getAlarmCounter());
        
        if(receiveSupervisionFrame(fd, UA)>=0){
            resetAlarm();
            return 0;
        }
        
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3);
    resetAlarm();

    perror("Error retriving supervision frame\n");

    
    llclose(fd,TRANSMITTER);
    

    exit(-1);
}


int transmitterApp(char *path, int fd){
    int fileFd;
    struct stat fileStat;

    //printf("Before lstat\n");

    if (lstat(path, &fileStat)<0){
        perror("Error getting file information.\n");
        return -1;
    }

    //printf("lstat successful\n");

    if ((fileFd = open(path, O_RDONLY)) < 0){
        perror("Error opening file.\n");
        return -1;
    }
    
   // printf("gif opened successfully\n");

    //Generates and sends START control frame
    unsigned int L1 = sizeof(fileStat.st_size);  //Size of file

    //printf("L1\n");
    
    unsigned int L2 = strlen(path);  //Length of file name
    //printf("L2\n");
    unsigned int frameSize = 5 + L1 + L2;

    unsigned char *controlFrame = buildControlFrame(START_FRAME, fileStat.st_size, path, L1, L2, frameSize);

    //printf("built control frame\n");
    int resp;
    while((resp=llwrite(fd, controlFrame, frameSize))==-1){
        usleep(STOP_AND_WAIT);
    }
    if(resp==-2){
        perror("Error sending START frame.\n");
        free(controlFrame);
        return -1;
    }

    //printf("wrote start frame sucessfully\n");

    usleep(STOP_AND_WAIT);
    //Generates and sends data packets
    char *buf=(char*)malloc(sizeof(char)*MAX_SIZE);
    unsigned int bytesToSend, noBytes;
    unsigned int sequenceNumber = 0;

    while((noBytes = read(fileFd, buf, MAX_SIZE-4))){
        bytesToSend = noBytes + 4;
        unsigned char *data=(unsigned char *)malloc(sizeof(unsigned char)*bytesToSend);
        //printf(" ------Data size------ %d\n",noBytes);
        data[0] = DATA;
        data[1] = sequenceNumber % 255;
        data[2] = noBytes /256;
        data[3] = noBytes % 256;
        memcpy(&data[4], buf, noBytes);
        while((resp=llwrite(fd, data, bytesToSend))==-1){
            usleep(STOP_AND_WAIT);
        }
        if(resp==-2){
            perror("Error sending Data frames\n");
            free(data);
            free(controlFrame);
            return -1;
        }
        free(data);
        sequenceNumber++;
        usleep(STOP_AND_WAIT);
    }
    free(buf);
    printf("Number of data frames sent: %d\n", sequenceNumber);

    usleep(STOP_AND_WAIT);
    //Generates and sends END control frame
    unsigned char *endControlFrame = buildControlFrame(END_FRAME, fileStat.st_size, path, L1, L2, frameSize);
    //printf("frame size- %d\n",frameSize);

    while((resp=llwrite(fd, endControlFrame, frameSize))==-1){
        usleep(STOP_AND_WAIT);
    }
    if(resp==-2){
        perror("Error sending END frame.\n");
        free(controlFrame);
        free(endControlFrame);
        return -1;
    }

    
    free(controlFrame);
    free(endControlFrame);
    return 0;
}


int llwrite(int fd, unsigned char* buffer,int length){

    //Init
    printf("\nMessage: %x\n", buffer);

    infoFrame frame = messageStuffing(buffer, length);
    printInfoFrame(frame);
    //printf("raw data %hhn\n",frame.rawData);
    int size;
    //printf("raw size %d\n",frame.rawSize);
    alarm(ALARM_TIME);
    //printf("Alarm counter %d\n",getAlarmCounter());
    do{
        if((size=write(fd, frame.rawData, frame.rawSize))>=0){
            printf("Message sent\n");     
        }     
        else{
            alarm(0);
            setAlarmFlag(0);
            printf("Message not sent\n");
            free(frame.rawData);
            free(frame.data);
            return -1;  
        }
        
        if(getAlarmFlag()){
            alarm(ALARM_TIME);
            setAlarmFlag(0);
            //printf("Alarm counter %d\n",getAlarmCounter());
        }
        

        unsigned char response = readSupervisionFrame(fd);
        if(response==0xff)
            continue;

        if(response==CONTROL_RJ(1)||response==CONTROL_RJ(0)){
            alarm(0);
            setAlarmFlag(0);
            printf("Negative response\n");
            free(frame.rawData);
            free(frame.data);
            return -1;
        }
        else if (response==CONTROL_RR(currFrame)){
            resetAlarm();
            if(currFrame)
                currFrame--;
            else
                currFrame++;    
            
            free(frame.rawData);
            free(frame.data);
            return size;
        }
    }while (getAlarmCounter()<RT_ATTEMPTS);
    resetAlarm();
    free(frame.rawData);
    free(frame.data);
    return -2;
}



 infoFrame messageStuffing(unsigned char* buff, int length){
    infoFrame frame;
    memset(&frame,0, sizeof( infoFrame));
    frame.flag=FLAG;
    frame.address=A;
    frame.control=CONTROL_I(currFrame);
    frame.bcc1=frame.address ^ frame.control;
    frame.bcc2=0xff;
    int size=length;
    frame.data=(unsigned char*)malloc((size+1)*sizeof(unsigned char));
    frame.size=0;
    for(int i =0; i<length;i++){
        if(buff[i]==ESC){
            frame.data=(unsigned char*)realloc(frame.data, ++size);
            frame.data[frame.size++]=ESC;
            frame.data[frame.size++]=ESC_ESC;
        }
        else if(buff[i]==FLAG){
            frame.data=(unsigned char*)realloc(frame.data, ++size);
            //printf("got Flag\n");
            frame.data[frame.size++]=ESC;
            frame.data[frame.size++]=ESC_FLAG;
        }
        else
            frame.data[frame.size++]=buff[i];
        
        frame.bcc2=buff[i]^frame.bcc2;
    }
    frame.rawData=(unsigned char*)malloc((frame.size+7)*sizeof(unsigned char));
    frame.rawData[0]=frame.flag;
    frame.rawData[1]=frame.address;
    frame.rawData[2]=frame.control;
    frame.rawData[3]=frame.bcc1;

    for (int i=0; i< frame.size;i++){
        frame.rawData[i+4]=frame.data[i];
        //printf("%x\n",frame.data[i]);
    }
    
    if(frame.bcc2==ESC){
        frame.rawData[frame.size+4]=frame.bcc2;
        frame.rawData[frame.size+5]=ESC_ESC;
        frame.rawData[frame.size+6]=frame.flag;
        frame.rawSize=frame.size+7;
    }
    else if (frame.bcc2==FLAG){
        frame.rawData[frame.size+4]=ESC;
        frame.rawData[frame.size+5]=ESC_FLAG;
        frame.rawData[frame.size+6]=frame.flag;
        frame.rawSize=frame.size+7;
    }
    else{
        frame.rawData[frame.size+4]=frame.bcc2;
        frame.rawData[frame.size+5]=frame.flag;
        frame.rawSize=frame.size+6;
    }
    //printf("----Frame Size -----%d\n",frame.size);
    //printf("------Raw Size -----%d\n",frame.rawSize);
    //for(int i=0; i<frame.rawSize;i++)
    //printf("Raw data %d: 0x%x\n",i,frame.rawData[i]);
    return frame;
}





int closeWriter(int fd){

    //printf("Closing writer...\n");


    do{
        sendSupervisionFrame(fd,DISC);
        //printf("sent DISC frame\n")
        alarm(ALARM_TIME);
        setAlarmFlag(0);
        //printf("%d\n",getAlarmCounter());
        
        if(receiveSupervisionFrame(fd, DISC)){
            sendSupervisionFrame(fd,UA);
            alarm(0);
            setAlarmCounter(0);
            printf("\nClosed Writer\n");
            return 0;
        }
        
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3);
    setAlarmCounter(0);



    printf("Error recieving DISC Frame...\n");   

    return -1;
}






/* End of Writer App*/




