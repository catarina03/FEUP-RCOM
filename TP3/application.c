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
        //sendSupervisionFrame(fd, closeUA.control, closeUA.bcc);
    }


    return TRUE;
}


int closeReader(int fd){

    printf("Closing reader...\n");
    if (receiveSupervisionFrame(fd, DISC) < 0) return -1;

    //Builds supervision frame
    //supervisionFrame close;
    //buildSupervisionFrame(&close, DISC);

    //sendSupervisionFrame(fd, close.control, close.bcc);

    sendMessage(fd,DISC);

    if (receiveSupervisionFrame(fd, UA) < 0) return -1;


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
      len = rawBytes[++i];
      frame.fileSize = (unsigned char*) malloc(len);

      for (int j = 0; i < i+len;j++) {
        frame.fileSize[j] = rawBytes[++i];
        frame.filesizeSize++;
      }
      
    }
    else if (rawBytes[i] == FILE_NAME) {
      len = rawBytes[++i];
      
      frame.fileName = (unsigned char *) malloc (len+1);
      
      for (int j = 0; i < i+len;j++) {
        frame.fileName[j] = rawBytes[++i];
        frame.filenameSize++;
      }
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

    printf("L1\n");
    
    unsigned int L2 = strlen(path);  //Length of file name
    printf("L2\n");
    unsigned int frameSize = 5 + L1 + L2;

    unsigned char *controlFrame = buildControlFrame(START_FRAME, fileStat.st_size, path, L1, L2, frameSize);

    printf("built control frame\n");

    if(llwrite(fd, controlFrame, frameSize) < 0){
        perror("Error sending START packet.");
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
            perror("llwrite failed");
            return -1;
        }

        sequenceNumber++;
    }

    printf("Number of data packets sent: %d\n", sequenceNumber);


    //Generates and sends END control frame
    unsigned char *endControlFrame = buildControlFrame(END_FRAME, fileStat.st_size, path, L1, L2, frameSize);

    if(llwrite(fileFd, endControlFrame, frameSize) < 0){
        perror("Error sending END packet.");
        return -1;
    }

    return 0;
}


void print_control_packet(controlFrame control){
    printf("--- Control packet ---\n");
    printf("control: %x\n", control.control);
    printf("fileSize: %s\n", control.fileSize);
    printf("fileName: %s\n", control.fileName);
    printf("filesize size: %d\n", control.filesizeSize);
    printf("filename size: %d\n", control.filenameSize);
    printf("raw bytes: %x\n", control.rawBytes);
    printf("raw size: %d\n", control.rawSize);

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


data_packet_t parse_data_packet(unsigned char *raw_bytes, int size) {
  data_packet_t packet;
  memset(&packet, 0, sizeof(data_packet_t));
  packet.raw_bytes_size = size;
  packet.control = raw_bytes[0];
  packet.sequence = raw_bytes[1];

  packet.data_field_size = (raw_bytes[2] << 8) | raw_bytes[3];

  for (int i = 0; i < packet.data_field_size; i++) {
    packet.data[i] = raw_bytes[4 + i];
  }

  return packet;
}


void print_data_packet(data_packet_t* packet, int full_info) {
  printf("---- DATA PACKET ----\n");
  printf("Control: - (0x%x)\n", packet->control);
  printf("Data size: %d (0x%x)\n", packet->data_field_size,
         packet->data_field_size);
  printf("Sequence: %d (0x%x)\n", packet->sequence, packet->sequence);

  if (full_info) {
    for (int i = 0; i < packet->data_field_size; i++) {
      printf("DATA[%d]: %c (0x%x)\n", i, packet->data[i], packet->data[i]);
    }
  }

  printf("---------------------\n");
}

int receiverApp(int fd){

    char buff[1024];
    int size;

    int state = 0;


    //START Control Packet
    while (!state) {
        memset(buff, 0, sizeof(buff));
        while ((size = llread(fd, buff)) < 0) {
            printf("Error reading\n");
            llclose(fd, RECEIVER);
            return -1;
        }
        controlFrame frame= parseControlFrame(buff,size);

        unsigned long fileSize =0;
        for(int i=0; i<frame.filesizeSize;i++)
            fileSize|=frame.fileSize[i]<<(8*i);
        
        
        file.name = frame.fileName;

        print_control_packet(frame);
        if (frame.control == START_FRAME) 
            state = 1;
        
    }


    // * DATA Packets
    unsigned char *fullMessage = (unsigned char*) malloc (file.size);
    int index = 0;
    int current_sequence = -1;

    while (state == 1) {
        memset(buff, 0, sizeof(buff));
        while ((size = llread(fd, buff)) <0) {
            printf("Error reading\n");
        }
        if (buff[0] == END_FRAME) {
            state = 2;
            break;
        }
        data_packet_t data = parse_data_packet(buff, size);
        
        if (data.control != DATA) continue;
        
        print_data_packet(&data, FALSE);
        for (int i =0;i<data.data_field_size;i++){
            data.data[index+i] = fullMessage[i];
        }

        // * caso o numero de sequencia seja diferente do anterior deve atualizar o index
        if (current_sequence != data.sequence) {
            current_sequence = data.sequence;
            index += data.data_field_size;
        }
    }


    // * STOP Control Packet
    if (state == 2) {
        controlFrame frame = parseControlFrame(buff, size);
        print_control_packet(frame);

        char* name = (char*) malloc ((strlen(file.name) + 7) * sizeof(char));
        sprintf(name, "cloned_%s", file.name);
        
        FILE *fl = fopen(name, "wb");
        if (fl != NULL) {
            fwrite (fullMessage, sizeof (unsigned char), fl->size, fl);
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



    infoFrame frame = messageDestuffing(buffer,fd);


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
    for(int i=0; i<frame.rawSize;i++)
    printf("Raw data %d: 0x%x\n",i,frame.rawData[i]);
    return frame;
}

infoFrame messageDestuffing(char* buff,int fd){

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
            printf("Flag 1\n");
            continue;
        }
        else if (msg == FLAG && flags == 1) {
            flags = 2;
            printf("Flag 2\n");
            break;
        }
        frame.rawData[size-extra] = msg;
        frame.rawData = (unsigned char*) realloc (frame.rawData, (size-extra+1));

        
        if (size>0 && frame.rawData[size-extra] == ESC) {
            
            if (frame.rawData[size-extra] == ESC_ESC){
                extra++;           
            }
            else if (frame.rawData[size-extra+1] == ESC_FLAG){
                frame.rawData[size-extra]=FLAG;
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

