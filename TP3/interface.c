#include "interface.h"


int main(int argc, char **argv){
    //set_alarm();
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
                        app.port = argv[2];
                    }
                    break;
                case 3:
                    if (strcmp(argv[3], "-r")!=0 && strcmp(argv[3], "-w")!=0){
                        printf("Usage: ./application -p <port> -r/-w <file_path>\n");
                        printf("tag -r/-w is missing\n");
                        return -1;
                    }
                    else{
                        if (strcmp(argv[3], "-r") == 0){
                            app.status = RECEIVER;
                            app.path = argv[4];
                        }
                        if (strcmp(argv[3], "-w") == 0){
                            app.status = TRANSMITTER;
                            app.path = argv[4];
                        } 
                    }
                    break;
            }
        }
    }

    if((app.fileDescriptor=llopen(app.port, app.status)) < 0){
        printf("Error opening file descriptor\n");
        exit(1);
    }


    
    if(app.status== TRANSMITTER){
        transmitterApp(app.path,app.fileDescriptor);

    }




    
    else if(app.status==RECEIVER){
        if(receiverApp(app.fileDescriptor)<0){
            perror("Error on receiver\n");
            exit(1);
        }





    }


    printf("About to close\n");

    if(llclose(app.fileDescriptor,app.status)){
        printf("Error closing file descriptor");
        exit(1);

    }



    return 0;
}