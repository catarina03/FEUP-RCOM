#include "interface.h"


int main(int argc, char **argv){

    applicationLayer app;

    //parses arguments
    if (argc != 5){
        printf("Usage: ./application -p <port> -r/-w <file_path>\n");
    }
    else{
        for (int i = 0; i < argc; i++){
            switch(i){
                case 1:
                    if (strcmp(argv[1], "-p")!=0){
                        printf("Usage: ./application -p <port> -r/-w <file_path>\n");
                        printf("-p tag is missing\n");
                        return -1;
                    }
                    break;
                case 2:
                    if ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) &&
                        (strcmp("/dev/ttyS10", argv[2])!=0) && (strcmp("/dev/ttyS11", argv[2])!=0) ){
                        printf("Usage: ./application -p <port> -r/-w <file_path>\n");
                        printf("port is missing\n");
                        return -1;
                    }
                    else{
                        app.fileDescriptor = argv[2];
                    }
                    break;
                case 3:
                    if (strcmp(argv[3], "-r")!=0 && strcmp(argv[3], "-w")!=0){
                        printf("Usage: ./application -p <port> -r/-w <file_path>\n");
                        printf("tag -r/-w is missing", argv[3]);
                        return -1;
                    }
                    else{
                        if (strcmp(argv[3], "-r") == 0) app.status = RECEIVER;
                        if (strcmp(argv[3], "-w") == 0) app.status = TRANSMITTER;
                    }
                    break;
            }
        }
    }

    llopen(app.fileDescriptor, app.status);

    if(app.status== TRANSMITTER){
        
          




    }
    else if(app.status==RECEIVER){

        if(!receiveMessage(app.fileDescriptor,SET)){
            sendMessage(app.fileDescriptor,UA);
        }
        else{
            printf("Did not UA. exited program\n ");
            llclose(app.fileDescriptor,app.status);
            exit(-1);
        }

        
        char buff[256];
        if(llread(app.fileDescriptor,buff)==-1){
            printf("Error on readings");
        }

        

    }

    llclose(app.fileDescriptor,app.status);



    return 0;
}