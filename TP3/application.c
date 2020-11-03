#include "application.h"

static int currFrame=0;

static int alarmFlag = 1;

static int alarmCounter=0;

struct termios oldtio;


int openReader( int fd){

    //Builds supervision frame
    //supervisionFrame reply;
    //buildSupervisionFrame(&reply, UA);

    if(receiveSupervisionFrame(fd,SET) >= 0){
        //sendSupervisionFrame(fd,reply.control, reply.bcc);
        sendMessage(fd,UA);
    }
    else{
        printf("Did not UA. exited program\n ");
        llclose(fd, RECEIVER);
        exit(-1);
    }

    printf("Reader received SET frame and sent UA successfully\n");
    return 0;  
}


int openWriter(int fd){

    //Builds supervision frame
    //supervisionFrame start;
    //buildSupervisionFrame(&start, SET);
    
    //sendSupervisionFrame(fd, start.control, start.bcc);
    sendMessage(fd,SET);
    
    do{
        alarm(3);
        setAlarmFlag(0);
        //printf("%d\n",getAlarmCounter());
        while(!getAlarmFlag()){
            if(receiveSupervisionFrame(fd, UA)){
               return 0;
            }
        }
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3);
    setAlarmCounter(0);
    



    perror("Error retriving supervision frame\n");
    exit(-1);
}


int closeWriter(int fd){

    printf("Closing writer...\n");

    //supervisionFrame close;
    //buildSupervisionFrame(&close, DISC);

    //sendSupervisionFrame(fd, close.control, close.bcc);

    sendMessage(fd,DISC);

    printf("sent DISC frame\n");

    // closeUA;
    //buildSupervisionFrame(&closeUA, UA);

    if (receiveSupervisionFrame(fd, DISC)){
        printf("received DISC frame\n");
        sendMessage(fd,UA);
        return 0;
        //sendSupervisionFrame(fd, closeUA.control, closeUA.bcc);
    }


    return TRUE;
}


int closeReader(int fd){

    printf("Closing reader...\n");
    if (receiveSupervisionFrame(fd, DISC) < 0) {
        printf("Error recieving DISC Frame...\n"); 
        return -1;}

    //Builds supervision frame
    //supervisionFrame close;
    //buildSupervisionFrame(&close, DISC);

    //sendSupervisionFrame(fd, close.control, close.bcc);

    sendMessage(fd,DISC);

    if (receiveSupervisionFrame(fd, UA) < 0) {
        printf("Error recieving UA Frame...\n");   
        return -1;
    }

    return 0;
}


unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize) {
    unsigned char *frame=(unsigned char*) malloc(sizeof(unsigned char)*frameSize);

    frame[0] = ctrlField;
    frame[1] = FILE_SIZE;
    frame[2] = L1;
    memcpy(&frame[3], &fileSize, L1);
    frame[3+L1] = FILE_NAME;
    frame[4+L1] = L2;
    memcpy(&frame[5+L1], fileName, L2);

    return frame;
}





controlFrame parseControlFrame(unsigned char *rawBytes, int size) {
  controlFrame frame;
  memset(&frame, 0, sizeof(controlFrame));
  frame.control = rawBytes[0];

  frame.filenameSize = 0;

  frame.filesizeSize = 0;
  int len;
  for (int i = 1; i < size; i++) {
    if (rawBytes[i] == FILE_SIZE) {
         printf("Parsing file size\n");
      len = rawBytes[++i];
      printf("len %d\n",len);
      frame.fileSize = (unsigned char*) malloc(len);

      for (int j = 0; j <len;j++) {
        frame.fileSize[j] = rawBytes[++i];
        printf("j %d\n",j);
        
      }
      frame.filesizeSize=len;
      
    }
    else if (rawBytes[i] == FILE_NAME) {
        printf("Parsing file name\n ");
      len = rawBytes[++i];
      
      frame.fileName = (unsigned char *) malloc (len+1);
      
      for (int j = 0; j < len;j++) {
        frame.fileName[j] = rawBytes[++i];
      }
      frame.filenameSize=len;
    }
  }

  frame.rawSize=size;
  frame.fileName[frame.filenameSize] = '\0';
  
  frame.rawBytes=rawBytes;
  
  
   
  return frame;
}




int transmitterApp(char *path, int fd){
    int fileFd;
    struct stat fileStat;

    printf("Before lstat\n");

    if (lstat(path, &fileStat)<0){
        perror("Error getting file information.");
        return -1;
    }

    printf("lstat successful\n");

    if ((fileFd = open(path, O_RDONLY)) < 0){
        perror("Error opening file.");
        return -1;
    }
    
    printf("gif opened successfully\n");

    //Generates and sends START control frame
    unsigned int L1 = sizeof(fileStat.st_size);  //Size of file

    //printf("L1\n");
    
    unsigned int L2 = strlen(path);  //Length of file name
    //printf("L2\n");
    unsigned int frameSize = 5 + L1 + L2;

    unsigned char *controlFrame = buildControlFrame(START_FRAME, fileStat.st_size, path, L1, L2, frameSize);

    printf("built control frame\n");

    if(llwrite(fd, controlFrame, frameSize) < 0){
        perror("Error sending START frame.");
        return -1;
    }

    printf("wrote start frame sucessfully\n");

    
    //Generates and sends data packets
    char buf[MAX_SIZE];
    unsigned int bytesToSend, noBytes;
    unsigned int sequenceNumber = 0;

    while(noBytes = read(fileFd, buf, MAX_SIZE)){
        unsigned char data[MAX_SIZE];
        data[0] = DATA;
        data[1] = sequenceNumber % 255;
        data[2] = noBytes / 256;
        data[3] = noBytes % 256;
        memcpy(&data[4], buf, noBytes);

        if((noBytes + 4) < MAX_SIZE)
            bytesToSend = noBytes + 4;
        else
            bytesToSend = MAX_SIZE;
        

        if(llwrite(fd, data, bytesToSend) < 0){
            perror("Error sending Data frames\n");
            return -1;
        }

        sequenceNumber++;
    }

    printf("Number of data frames sent: %d\n", sequenceNumber);


    //Generates and sends END control frame
    unsigned char *endControlFrame = buildControlFrame(END_FRAME, fileStat.st_size, path, L1, L2, frameSize);
    printf("frame size- %d\n",frameSize);
    if(llwrite(fileFd, endControlFrame, frameSize) < 0){
        perror("Error sending END frame.");
        return -1;
    }

    return 0;
}


void printControlFrame(controlFrame frame){
    printf("       CONTROL      \n");
    printf("Control: %x\n", frame.control);
    printf("File size: %s\n", frame.fileSize);
    printf("File name: %s\n", frame.fileName);
    printf("Filesize size: %d\n", frame.filesizeSize);
    printf("Filename size: %d\n", frame.filenameSize);
    printf("Raw bytes: %x\n", frame.rawBytes);
    printf("Raw size: %d\n", frame.rawSize);

/*
      unsigned char control;      
  unsigned char *fileSize;   
  unsigned char *fileName;   
  int filesizeSize; 
  int filenameSize;

  unsigned char *rawBytes;   
  int rawSize; 
  */
    
}


dataFrame parseDataFrame(unsigned char *rawBytes, int size) {
  dataFrame frame;
  memset(&frame, 0, sizeof(dataFrame));
  frame.rawBytes = size;
  frame.control = rawBytes[0];
  frame.sequence = rawBytes[1];

  frame.dataSize = (rawBytes[2] << 8) | rawBytes[3];

  for (int i = 0; i < frame.dataSize; i++) {
    frame.data[i] = rawBytes[i+4];
  }

  return frame;
}


void printDataFrame(dataFrame* frame, int full) {
  printf("     DATA     \n");
  printf("Control: - (0x%x)\n", frame->control);
  printf("Data size: %d (0x%x)\n", frame->dataSize,
         frame->dataSize);
  printf("Sequence: %d (0x%x)\n", frame->sequence, frame->sequence);

  if (full) {
    for (int i = 0; i < frame->dataSize; i++) {
      printf("DATA[%d]: %c (0x%x)\n", i, frame->data[i], frame->data[i]);
    }
  }

  printf("\n");
}

int receiverApp(int fd){

    char buff[1024];
    int size;

    int state = 0;

    unsigned char* fileData;
    
    unsigned char* fileName;

    unsigned long fileSize=0;
    //START Control Packet
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
        
        
        fileName = frame.fileName;

        printControlFrame(frame);
        if (frame.control == START_FRAME) 
            state = 1;
        
    }


    // * DATA Packets
    unsigned char *fullMessage = (unsigned char*) malloc (fileSize);
    int index = 0;
    int currSequence = -1;

    while (state == 1) {
        memset(buff, 0, sizeof(buff));
        while ((size = llread(fd, buff)) <0) {
            printf("Error reading\n");
        }
        if (buff[0] == END_FRAME) {
            printf("Recieved End Frame");
            state = 2;
            break;
        }
        dataFrame data = parseDataFrame(buff, size);
        
        if (data.control != DATA) continue;
        
        printDataFrame(&data, FALSE);
        for (int i =0;i<data.dataSize;i++){
            data.data[index+i] = fullMessage[i];
        }

        // * caso o numero de sequencia seja diferente do anterior deve atualizar o index
        if (currSequence != data.sequence) {
            currSequence = data.sequence;
            index += data.dataSize;
        }
    }


    // * STOP Control Packet
    if (state == 2) {
        controlFrame frame = parseControlFrame(buff, size);
        printControlFrame(frame);

        char* name = (char*) malloc ((strlen(fileName) ) * sizeof(char));
        //sprintf(name, "cloned_%s", fileName); //^ add +7 
        
        FILE *fl = fopen(name, "write_obj");
        if (fl != NULL) {
            fwrite (fullMessage, sizeof (unsigned char), fileSize, fl);
            fclose (fl);
        }
        printf("Received file\n");
    }

    // resets and closes the receiver fd for the port
    llclose(fd, RECEIVER);

    return 0;
}






/*
void buildSupervisionFrame(supervisionFrame *frame, unsigned char controlByte){
  frame->flag = FLAG;
  frame->address = A;
  frame->control = controlByte;
  frame->bcc = frame->address ^ frame->control;
}*/


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
        return closeReader(fd);
    }
    else if(type == TRANSMITTER){
        return closeWriter(fd);
    }

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
        perror("tcsetattr");
        exit(-1);
    }

    return -1;
}





int llwrite(int fd, char* buffer,int length){

    //Init
    printf("Message: %x\n", buffer);

    infoFrame frame = messageStuffing(buffer, length);

    int size;
    printf("raw size %d\n",frame.rawSize);
    if((size=write(fd, frame.rawData, frame.rawSize))>=0)
        printf("Message sent\n");      
    else{
        printf("raw data %x\n",frame.rawData);
        //printf("raw size %d\n",frame.rawSize);
        //printf("size- %d",size);
        printf("Message not sent\n");
        return -1;  
    }
    
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



    infoFrame frame = messageDestuffing(buffer,fd);

    printf("Exited destuffing\n");
    if(frame.size>0)
        memcpy(buffer, frame.data,frame.size);
    
    //printf("bcc1: %x - %x address ^ control\n",frame.bcc1,frame.address ^frame.control);
    if(frame.bcc1!=(frame.address ^frame.control) ){
        sendMessage(fd, CONTROL_RJ(currFrame));
        printf("Sent Negative Response\n");
        return -1;
    }
    else{
        sendMessage(fd, CONTROL_RR(currFrame));
        printf("Sent Positive Response\n");
        if(currFrame)
            currFrame--;
        else
            currFrame++;    
            
        return frame.size;
    }



}






infoFrame messageStuffing(char* buff, int length){
    infoFrame frame;
    memset(&frame,0, sizeof(infoFrame));
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

    frame.rawData=(unsigned char*)malloc((frame.size+8)*sizeof(unsigned char));
    frame.rawData[0]=frame.flag;
    frame.rawData[1]=frame.address;
    frame.rawData[2]=frame.control;
    frame.rawData[3]=frame.bcc1;

    for (int i=0; i< frame.size;i++){
        frame.rawData[i+4]=frame.data[i];
        //printf("%x\n",frame.data[i]);
    }
    
    if(frame.bcc2==ESC){
        frame.rawData[frame.size+5]=frame.bcc2;
        frame.rawData[frame.size+6]=ESC_ESC;
        frame.rawData[frame.size+7]=frame.flag;
        frame.rawSize=frame.size+8;
    }
    else if (frame.bcc2=FLAG){
        frame.rawData[frame.size+5]=ESC;
        frame.rawData[frame.size+6]=ESC_FLAG;
        frame.rawData[frame.size+7]=frame.flag;
        frame.rawSize=frame.size+8;
    }
    else{
        frame.rawData[frame.size+5]=frame.bcc2;
        frame.rawData[frame.size+6]=frame.flag;
        frame.rawSize=frame.size+7;
    }
    //for(int i=0; i<frame.rawSize;i++)
    //printf("Raw data %d: 0x%x\n",i,frame.rawData[i]);
    return frame;
}

infoFrame messageDestuffing(char* buff,int fd){

    infoFrame frame;
    frame.rawData=(unsigned char*) malloc (sizeof(unsigned char));
    int flags = 0;
    unsigned char msg;
    int size=0;
    while(flags!=2){
        read(fd, &msg, 1);

        
        if (msg == FLAG && flags == 0) {
            flags = 1;
            //printf("Flag 1\n");
            continue;
        }
        else if (msg == FLAG && flags == 1) {
            flags = 2;
            //printf("Flag 2 - size %d\n",size);
            break;
        }
        frame.rawData[size] = msg;
        frame.rawData = (unsigned char*) realloc (frame.rawData, (size+1));
        
        if (size>0 && frame.rawData[size-1] == ESC) {
            if (frame.rawData[size] == ESC_FLAG)
                frame.rawData[--size]=FLAG;
            else if (frame.rawData[size] == ESC_ESC)
                  size--;
            
        }
        size++;
    }
    frame.flag=FLAG;
    frame.address = frame.rawData[0];
    frame.control = frame.rawData[1];
    frame.bcc1 = frame.rawData[2];
    frame.data = (unsigned char*) malloc(sizeof(unsigned char)*(size-3));
    printf("%d\n",size);
    for (int i = 3; i < size - 1; i++) {
        frame.data[i-3] = frame.rawData[i];
        
        
    }
    frame.bcc2=frame.rawData[size-1];
    frame.size=size-4;
    return frame;
    
}



void printInfoFrame(infoFrame frame){
    
    


}

