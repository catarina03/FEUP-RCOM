#include "application.h"

static int currFrame=0;

struct termios oldtio;


int openReader( int fd){



    if(receiveSupervisionFrame(fd,SET) >= 0){
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

    
    llclose(fd,TRANSMITTER);
    

    exit(-1);
}


int closeWriter(int fd){

    printf("Closing writer...\n");


    sendMessage(fd,DISC);

    printf("sent DISC frame\n");



    if (receiveSupervisionFrame(fd, DISC)){
        printf("received DISC frame\n");
        sendMessage(fd,UA);
        return 0;
        
    }


    return TRUE;
}


int closeReader(int fd){

    printf("Closing reader...\n");
    if (receiveSupervisionFrame(fd, DISC) < 0) {
        printf("Error recieving DISC Frame...\n"); 
        return -1;}

    sendMessage(fd,DISC);

    if (receiveSupervisionFrame(fd, UA) < 0) {
        printf("Error recieving UA Frame...\n");   
        return -1;
    }
    printf("Closed Reader\n");
    return 0;
}


unsigned char *buildControlFrame(char ctrlField, unsigned fileSize, char* fileName, unsigned int L1, unsigned int L2, unsigned int frameSize) {
    unsigned char *frame=(unsigned char*) malloc(sizeof(unsigned char)*frameSize);
    printf(" ----Frame Size -------%d\n",frameSize);
    frame[0] = ctrlField;
    frame[1] = FILE_SIZE;
    frame[2] = L1;
    memcpy(&frame[3], &fileSize, L1);
    frame[3+L1] = FILE_NAME;
    frame[4+L1] = L2;
    memcpy(&frame[5+L1], fileName, L2);
    printf(" -----File Size -------%d\n",fileSize);
    printf(" -----Size Size -------%d\n",L1);
    printf(" -----File Name -------%s\n",fileName);
    printf(" -----Name Size -------%d\n",L2);

    return frame;
}





controlFrame parseControlFrame(unsigned char *rawBytes, int size) {
  controlFrame frame;
  memset(&frame, 0, sizeof(controlFrame));
  frame.control = rawBytes[0];

  frame.filenameSize = 0;

  frame.filesizeSize = 0;
  int len;
  int fileSizeFlag=1;
  for (int i = 1; i < size;i++) {
    if (rawBytes[i] == FILE_SIZE && fileSizeFlag) {
        fileSizeFlag=0;
        // printf("Parsing file size\n");
      len = rawBytes[++i];
      //printf("len %d\n",len);
      frame.fileSize = (unsigned char*) malloc(len*sizeof(unsigned char));

      for (int j = 0; j <len;j++) {
        frame.fileSize[j] = rawBytes[++i];
        //printf("j %d - byte 0x%02x\n",j,frame.fileSize[j]);
        
      }
      frame.filesizeSize=len;
      
    }
    else if (rawBytes[i] == FILE_NAME) {
        //printf("Parsing file name\n ");
      len = rawBytes[++i];
      //printf("len %d\n",len);
      frame.fileName = (unsigned char *) malloc ((len+1)*sizeof(unsigned char));
      
      for (int j = 0; j < len;j++) {
        frame.fileName[j] = rawBytes[++i];
        //printf("j %d - byte 0x%02x\n",j,frame.fileName[j]);
      }
      frame.filenameSize=len;
    }
    
  }
  /*for (int i=1;i<size;i++)
    printf("RawData byte %d- 0x%02x\n",i,rawBytes[i]);*/

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

    if(llwrite(fd, controlFrame, frameSize) < 0){
        perror("Error sending START frame.\n");
        free(controlFrame);
        return -1;
    }

    //printf("wrote start frame sucessfully\n");

    
    //Generates and sends data packets
    char *buf=(char*)malloc(sizeof(char)*MAX_SIZE);
    unsigned int bytesToSend, noBytes;
    unsigned int sequenceNumber = 0;

    while((noBytes = read(fileFd, buf, MAX_SIZE-4))){
        bytesToSend = noBytes + 4;
        unsigned char *data=(unsigned char *)malloc(sizeof(unsigned char)*bytesToSend);
        printf(" ------Data size------ %d\n",noBytes);
        data[0] = DATA;
        data[1] = sequenceNumber % 255;
        data[2] = noBytes /256;
        data[3] = noBytes % 256;
        memcpy(&data[4], buf, noBytes);
        

        if(llwrite(fd, data, bytesToSend) < 0){
            perror("Error sending Data frames\n");
            free(data);
            free(controlFrame);
            return -1;
        }
        free(data);
        sequenceNumber++;
    }
    free(buf);
    printf("Number of data frames sent: %d\n", sequenceNumber);


    //Generates and sends END control frame
    unsigned char *endControlFrame = buildControlFrame(END_FRAME, fileStat.st_size, path, L1, L2, frameSize);
    printf("frame size- %d\n",frameSize);
    if(llwrite(fd, endControlFrame, frameSize) < 0){
        perror("Error sending END frame.\n");
        free(controlFrame);
        free(endControlFrame);
        return -1;
    }

    
    free(controlFrame);
    free(endControlFrame);
    return 0;
}


void printControlFrame(controlFrame frame){
    printf("       CONTROL      \n");
    printf("Control: %x\n", frame.control);
    printf("File size: %x\n", frame.fileSize);
    printf("File name: %s\n", frame.fileName);
    printf("Filesize size: %d\n", frame.filesizeSize);
    printf("Filename size: %d\n", frame.filenameSize);
    printf("Raw bytes: %x\n", frame.rawBytes);
    printf("Raw size: %d\n", frame.rawSize);
    
}


dataFrame parseDataFrame(unsigned char *rawBytes, int size) {
  dataFrame frame;
  memset(&frame, 0, sizeof(dataFrame));
  frame.rawBytes = rawBytes;
  frame.rawSize = size;
  frame.control = rawBytes[0];
  frame.sequence = rawBytes[1];

  frame.dataSize = (rawBytes[2] << 8) | rawBytes[3];

  for (int i = 0; i < frame.dataSize; i++) {
    frame.data[i] = rawBytes[i+4];
  }

  return frame;
}


void printDataFrame(dataFrame frame, int full) {
  printf("     DATA     \n");
  printf("Control: -0x%02x-\n", frame.control);
  printf("Data size: %d -0x%02x-\n", frame.dataSize,
         frame.dataSize);
  printf("Sequence: %d -0x%02x-\n", frame.sequence, frame.sequence);

  if (full) {
    for (int i = 0; i < frame.dataSize; i++) {
      printf("DATA[%d]: %x -0x%02x\n", i, frame.data[i], frame.data[i]);
    }
  }
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

    while (state == 1) {
        memset(buff, 0, sizeof(buff));
        //printf("done memset\n");
        while ((size = llread(fd, buff)) <0) {
            printf("Error reading\n");
        }
       // printf("exited llread\n");
        if (buff[0] == END_FRAME) {
            printf("Recieved End Frame\n");
            state = 2;
            break;
        }
        dataFrame data = parseDataFrame(buff, size);
        
        if (data.control != DATA) {
            continue;
            }
        
        //printDataFrame(&data, FALSE);
        for (int i =0;i<data.dataSize;i++){
            fullMessage[index+i] = data.data[i];
        }
        //printf("Printed Data frame\n");
        // * caso o numero de sequencia seja diferente do anterior deve atualizar o index
        if (currSequence != data.sequence) {
            currSequence = data.sequence;
            index += data.dataSize;
        }
    }


    // * END Control Frame
    if (state == 2) {
        controlFrame frame = parseControlFrame(buff, size);
        printControlFrame(frame);

        //char* name = (char*) malloc ((strlen(fileName) +7) * sizeof(char));
        char* name = (char*) malloc ((frame.filenameSize +7) * sizeof(char));
        //char name[frame.filenameSize +7];

        sprintf(name, "cloned_%s", frame.fileName); //^ add +7 
        
        FILE *fl = fopen(name, "wb");
        if (fl != NULL) {
            fwrite(fullMessage, sizeof (unsigned char), fileSize, fl);
            fclose(fl);
        }
        printf("Received file\n");
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
    printf("Freed everything\n");
    return 0;
}








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





int llwrite(int fd, unsigned char* buffer,int length){

    //Init
    printf("Message: %x\n", buffer);

    infoFrame frame = messageStuffing(buffer, length);
    //printInfoFrame(frame);
    //printf("raw data %hhn\n",frame.rawData);
    int size;
    //printf("raw size %d\n",frame.rawSize);
    if((size=write(fd, frame.rawData, frame.rawSize))>=0){
    //printf("Message sent\n");     
    }     
    else{
        printf("Message not sent\n");
        free(frame.rawData);
        free(frame.data);
        return -1;  
    }
    
    //stop and wait part
    usleep(STOP_AND_WAIT);

    unsigned char response = readSupervisionFrame(fd);

    if(response==CONTROL_RJ(1)||response==CONTROL_RJ(0)){
        printf("Negative response\n");
        free(frame.rawData);
        free(frame.data);
        return -1;
    }
    else{
        free(frame.rawData);
        free(frame.data);
        return size;
    }
}





int llread(int fd, char* buffer){

    infoFrame frame = messageDestuffing(buffer,fd);
    //printInfoFrame(frame);
    //printf("Exited destuffing\n");
    memcpy(buffer, frame.data,frame.size);
    
    //printf("bcc1: %x - %x address ^ control\n",frame.bcc1,frame.address ^frame.control);
    if(frame.bcc1!=(frame.address ^frame.control) ){
        sendMessage(fd, CONTROL_RJ(currFrame));
        printf("Sent Negative Response\n");
        free(frame.rawData);
        free(frame.data);
        return -1;
    }
    else{
        sendMessage(fd, CONTROL_RR(currFrame));
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






infoFrame messageStuffing(unsigned char* buff, int length){
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
    printf("----Frame Size -----%d\n",frame.size);
    printf("------Raw Size -----%d\n",frame.rawSize);
    //for(int i=0; i<frame.rawSize;i++)
    //printf("Raw data %d: 0x%x\n",i,frame.rawData[i]);
    return frame;
}

infoFrame messageDestuffing(unsigned char* buff,int fd){

    infoFrame frame;
    frame.rawData=(unsigned char*) malloc (sizeof(unsigned char));
    int flags = 0;
    unsigned char msg;
    int size=0;
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
            break;
        }
        //printf("msg- 0x%x size- %d rawData- %x",msg,size,frame.rawData);
        frame.rawData[size] = msg;
        frame.rawData = (unsigned char*) realloc (frame.rawData, (size+2));
        //printf(" --r");
        if (size>0 && frame.rawData[size-1] == ESC) {
            if (frame.rawData[size] == ESC_FLAG)
                frame.rawData[--size]=FLAG;
            else if (frame.rawData[size] == ESC_ESC)
                  size--;
            
        }
        size++;
        //printf("--new size-%d\n",size);
    }
    frame.rawData = (unsigned char*) realloc (frame.rawData, (size));
    //printf("exits\n");
    frame.flag=FLAG;
    frame.address = frame.rawData[0];
    frame.control = frame.rawData[1];
    frame.bcc1 = frame.rawData[2];
    frame.data = (unsigned char*) malloc(sizeof(unsigned char)*(size-3));
    //printf("%d\n",size);
    for (int i = 3; i < size - 1; i++) {
        frame.data[i-3] = frame.rawData[i];
        
        
    }
    frame.bcc2=frame.rawData[size-1];
    frame.size=size-4;
    frame.rawSize=size;
    return frame;
    
}



void printInfoFrame(infoFrame frame){
    printf("     Info     \n");
    printf("FLAG- 0x%x\n",frame.flag);
    printf("Address - 0x%x\n",frame.address);
    printf("Control - 0x%x\n",frame.control);
    printf("BCC1 - 0x%x",frame.bcc1);
    for(int i =0;i<frame.size;i++){
        if(i%8==0){
            printf("\nData - ");
        }
        printf(" 0x%x,",frame.data[i]);
    }
    printf("\nBCC2 - 0x%x\n",frame.bcc2);
    printf("Second FLAG- 0x%x",frame.flag);

    for(int i =0;i<frame.rawSize;i++){
        if(i%15==0){
            printf("\nRaw Data - ");
        }
        printf(" 0x%x,",frame.rawData[i]);
    }
    
    printf("\n");


}

