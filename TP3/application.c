#include "application.h"

static int currFrame=0;

static int fd;

static int alarmFlag = 1;

static int alarmCounter=0;

static int STOP=FALSE;

struct termios oldtio;


int openReader(char *port){

    //Builds supervision frame
    //supervisionFrame reply;
    //buildSupervisionFrame(&reply, UA);

    if(receiveSupervisionFrame(fd,SET) >= 0){
        //sendSupervisionFrame(fd,reply.control, reply.bcc);
        sendMessage(fd,UA);
        return TRUE;
    }
    else{
        printf("Did not UA. exited program\n ");
        llclose(fd, RECEIVER);
        exit(-1);
    }

    printf("Sent SET frame and received UA successfully\n");
    return FALSE;  
}


int openWriter(char *port){

    //Builds supervision frame
    //supervisionFrame start;
    //buildSupervisionFrame(&start, SET);
    
    //sendSupervisionFrame(fd, start.control, start.bcc);
    
    
    do{
        alarm(3);
        setAlarmFlag(0);
        while(!getAlarmFlag()){
            if((STOP=receiveSupervisionFrame(fd, UA))==TRUE){
               return TRUE;
            }
        }
        if(getAlarmFlag()){
            printf("Timed Out\n");
        } 
    }while(getAlarmCounter()<3 && STOP==FALSE);
    setAlarmCounter(0);
    




    return FALSE;
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
    unsigned char frame[frameSize];

    frame[0] = ctrlField;
    frame[1] = FILE_SIZE;
    frame[2] = L1;
    memcpy(&frame[3], &fileSize, L1);
    frame[3+L1] = FILE_NAME;
    frame[4+L1] = L2;
    memcpy(&frame[5+L1], fileName, L2);

    return frame;
}





unsigned char *parseControlFrame(unsigned char *raw_bytes, int size) {
  control_packet_t packet;
  memset(&packet, 0, sizeof(control_packet_t));
  packet.control = raw_bytes[0];

  char *name;
  int namesize = 0;

  unsigned char* filesize;
  int filesize_size = 0;

  for (int i = 1; i < size; i++) {
    if (raw_bytes[i] == FILE_SIZE) {
      int length = raw_bytes[++i];
      int offset = i + length;
      filesize = (unsigned char*) malloc (length);
      for (int k = 0; i < offset; k++) {
        filesize[k] = raw_bytes[++i];
        filesize_size++;
      }
      continue;
    }
    if (raw_bytes[i] == FILE_NAME) {
      int length = raw_bytes[++i];
      name = (unsigned char *) malloc (length);
      int offset = i + length;
      for (int j = 0; i < offset;) {
        name[j++] = raw_bytes[++i];
        namesize++;
      }
      continue;
    }
  }

  packet.file_name = (unsigned char*) malloc (namesize + 1);
  memcpy(packet.file_name, name, namesize);
  packet.file_name[namesize] = '\0';
  free(name);

  packet.filesize_size = filesize_size;
  packet.file_size = (unsigned char*) malloc (filesize_size);
  memcpy(packet.file_size, filesize, filesize_size);
  free(filesize);
   
  return packet;
}




int transmitterApp(char *path){
    int file_fd;
    struct stat file_stat;

    if (stat(path, &file_stat)<0){
        perror("Error getting file information.");
        return -1;
    }

    if ((file_fd = open(path, O_RDONLY)) < 0){
        perror("Error opening file.");
        return -1;
    }

    //Generates and sends START control frame
    unsigned int L1 = sizeof(file_stat.st_size);  //Size of file
    unsigned int L2 = strlen(*path);  //Length of file name
    unsigned int frame_size = 5 + L1 + L2;
    unsigned char controlFrame[frame_size] = buildControlFrame(START_FRAME, file_stat.st_size, path, L1, L2, frame_size)

    if(llwrite(file_fd, controlFrame, frame_size) < 0){
        perror("Error sending START packet.");
        return -1;
    }

    
    //Generates and sends data packets
    char buf[MAX_SIZE];
    unsigned int bytes_to_send, no_bytes;
    unsigned int sequenceNumber = 0;

    while(no_bytes = read(file_fd, buf, MAX_SIZE)){
        unsigned char data[MAX_SIZE];
        data[0] = DATA;
        data[1] = sequenceNumber % 255;
        data[2] = no_bytes / 256;
        data[3] = no_bytes % 256;
        memcpy(&data[4], buf, no_bytes);

        if((no_bytes + 4) < MAX_SIZE){
            bytes_to_send = no_bytes + 4;
        }
        else{
            bytes_to_send = MAX_SIZE;
        }

        if(llwrite(fd, data, bytes_to_send)){
            perror("llwrite failed");
            return -1;
        }

        sequenceNumber++;
    }

    printf("Number of data packets sent: %d\n", sequenceNumber);


    //Generates and sends END control frame
    unsigned char endControlFrame[frame_size] = buildControlFrame(END_FRAME, file_stat.st_size, path, L1, L2, frame_size)

    if(llwrite(file_fd, controlFrame, frame_size) < 0){
        perror("Error sending START packet.");
        return -1;
    }

    return 0;
}




int recieverApp(char *path){

    /* opens transmiter file descriptor on second layer */
    int receiver_fd = llopen(path, RECEIVER);
    /* in case there's an error oppening the port */
    if (receiver_fd <0>) {
        exit(-1);
    }


    char buffer[1024];
    int size;

    int state = 0;

    // * START Control Packet
    while (state == 0) {
        memset(buffer, 0, sizeof(buffer));
        while ((size = llread(receiver_fd, buffer)) == ERROR) {
        printf("Error reading\n");
        llclose(receiver_fd, RECEIVER);
        return ERROR;
        }
        unsigned char *frame

        file.size = array_to_number(packet.file_size, packet.filesize_size);
        file.name = packet.file_name;

        print_control_packet(&packet);
        if (packet.control == START) {
        state = 1;
        }
    }

    // * DATA Packets
    unsigned char *full_message = (unsigned char*) malloc (file.size);
    int index = 0;
    int current_sequence = -1;

    while (state == 1) {
        memset(buffer, 0, sizeof(buffer));
        while ((size = llread(receiver_fd, buffer)) == ERROR) {
        printf("Error reading\n");
        }
        if (buffer[0] == STOP) {
        state = 2;
        break;
        }
        data_packet_t data = parse_data_packet(buffer, size);
        
        if (data.control != DATA) continue;
        
        print_data_packet(&data, FALSE);
        join_file(full_message, data.data, data.data_field_size, index);

        // * caso o numero de sequencia seja diferente do anterior deve atualizar o index
        if (current_sequence != data.sequence) {
        current_sequence = data.sequence;
        index += data.data_field_size;
        }
    }

    // * STOP Control Packet
    if (state == 2) {
        control_packet_t packet = parse_control_packet(buffer, size);
        print_control_packet(&packet);

        char* name = (char*) malloc ((strlen(file.name) + 7) * sizeof(char));
        sprintf(name, "cloned_%s", file.name);
        write_file(name, full_message, file.size);
        printf("Received file\n");
    }

    /* resets and closes the receiver fd for the port */
    llclose(receiver_fd, RECEIVER);

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

    return -1;
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

