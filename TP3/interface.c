#include "interface.h"


int main(int argc, char **argv){

    applicationLayer app;

    //parses arguments
    if (argc != 5){
        printf("Usage: ./application -p <port> -r/-w <file_path>");
    }
    else{
        for (int i = 0; i < argc; i++){
            if (strcmp(argv[i], "./"
            switch(i){
                case 1:
                    if (strcmp(argv[1], "-p")!=0){
                        printf("Usage: ./application -p <port> -r/-w <file_path>");
                        return -1;
                    }
                case 2:
                    if ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) &&
                        (strcmp("/dev/ttyS10", argv[2])!=0) && (strcmp("/dev/ttyS11", argv[2])!=0) ){
                        printf("Usage: ./application -p <port> -r/-w <file_path>");
                        return -1;
                    }
                    else{
                        app.fileDescriptor = argv[2];
                    }
                case 3:
                    if (strcmp(argv[3], "-r")!=0 || strcmp(argv[3], "-w")!=0){
                        printf("Usage: ./application -p <port> -r/-w <file_path>");
                        return -1;
                    }
                    else{
                        if (strcmp(argv[3], "-r")) app.status = RECIEVER;
                        if (strcmp(argv[3], "-w")) app.status = TRANSMITTER;
                    }
            }
        }
    }

    llopen(app.fileDescriptor, app.status);



    return -1;
}