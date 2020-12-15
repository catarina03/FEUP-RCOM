#include "connection.h"


int init(char *ip, int port, int *socketfd){
    struct sockaddr_in servaddr; 
  
    // socket create and varification 
	if ((*socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Error creating socket");
		return 1;
  	} 
    bzero(&servaddr, sizeof(servaddr)); 

  	// server address handling
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	servaddr.sin_port = htons(port);		/*server TCP port must be network byte ordered */


	// connects to the server
  	if(connect(*socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    perror("Connection with the server failed");
		return 1;
	}

  	return 0;
}


int ftp_rcv_response(int socketfd){
    FILE* f = fdopen(socketfd,"r");

    char *buf;
    size_t bRead = 0;
    int response;

    while(getline(&buf,&bRead,f)>0){
        printf("%s",buf);
        if(buf[3]==' '){
            sscanf(buf,"%d",&response);
            break;
        }
    }
    return response;
}


int ftp_send_command(int sockfd, char *msg) {
    int b, len=strlen(msg);

    if((b=write(sockfd,msg,len))!=len){
        printf("Couldn't write to socket");
        return 1;
    }
    return 0;
}


//Log on Server
int ftp_login(int socketfd, char *username, char *password) {
	int ret;
	char cmd_send[1024];

    //Send USER
	sprintf(cmd_send, "USER %s\r\n", username);
	ret = ftp_send_command(socketfd, cmd_send);
	if(ret != 0)
	{	
		printf("ret is %d\n", ret);
		perror("Couldn't send USER command");
		socket_close(socketfd);
		return 1;
	}
	ret = ftp_rcv_response(socketfd);
	if(ret != USER_LOGIN)
	{
		perror("Couldn't log in user");
		socket_close(socketfd);
		return 1;
	}
	
    //Send PASS
	sprintf(cmd_send, "PASS %s\r\n", password);
	ret = ftp_send_command(socketfd, cmd_send);
	if(ret != 0)
	{
		socket_close(socketfd);
		return 1;
	}
	ret = ftp_rcv_response(socketfd);
	if(ret != PASS_LOGIN)
	{
		socket_close(socketfd);
		return 1;
	}
	
    //Set to binary mode
	ret = ftp_send_command(socketfd, "TYPE I\r\n");
	if(ret != 0)
	{
		perror("Couldn't send binary mode command");
		socket_close(socketfd);
		return 1;
	}
	ret = ftp_rcv_response(socketfd);
	if(ret != BIN_READY)
	{
		perror("Couldn't set binary mode");
		socket_close(socketfd);
		return 1;
	}
	return 0;
}



int socket_close(int sockfd) {
	close(sockfd);
	return 0;
}



