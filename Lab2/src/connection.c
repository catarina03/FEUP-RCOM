#include "connection.h"


//Inits connection
int init(char *ip, int port, int *socketfd){
    struct sockaddr_in servaddr; 

    bzero(&servaddr, sizeof(servaddr)); 

  	// server address handling
	bzero((char*)&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	servaddr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
  
    // socket create and varification 
	if ((*socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("Error creating socket");
		return 1;
  	} 

	// connects to the server
  	if(connect(*socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
    	perror("Connection with the server failed");
		return 1;
	}

  	return 0;
}


//Receives a response
int ftp_rcv_response(int socketfd){
    FILE* f = fdopen(socketfd,"r");
    char *buf;
    size_t bytes_read = 0;
    int response;

    while(getline(&buf,&bytes_read,f)>0){
        printf("%s",buf);
        if(buf[3]==' '){
            sscanf(buf,"%d",&response);
            break;
        }
    }

    return response;
}


//Sends a command
int ftp_send_command(int sockfd, char *msg) {
    int b, len=strlen(msg);

    if((b=write(sockfd,msg,len))!=len){
        printf("Couldn't write to socket");
        return 1;
    }

    return 0;
}


//Logs on Server
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
	if(ret != USER_LOGIN && ret != PASS_LOGIN)
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
		perror("Couldn't send PASS command");
		socket_close(socketfd);
		return 1;
	}
	ret = ftp_rcv_response(socketfd);
	if(ret != PASS_LOGIN)
	{
		perror("Couldn't send log in with password");
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


//Sets FTP server to passive mode
static int ftp_enter_passive(int socketfd, char *ip, int *port)
{
	int ret, a,b,c,d,pa,pb;
	char *find, *buf;

	ret = ftp_send_command(socketfd, "PASV\r\n");
	if(ret != 0)
	{
		perror("Couldn't send PASV command");
		socket_close(socketfd);
		return 1;
	}

	//Receives response
	FILE* f = fdopen(socketfd,"r");
    size_t bytes_read = 0;
    while(getline(&buf,&bytes_read,f)>0){
		printf("%s",buf);
        if(buf[3]==' '){
            sscanf(buf,"%d",&ret);
            break;
        }
    }
	if(ret != PASV_READY)
	{
		perror("Couldn't set passive mode");
		socket_close(socketfd);
		return 1;
	}

	//Calculates port
	find = strrchr(buf, '(');
	sscanf(find, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &pa, &pb);
	sprintf(ip, "%d.%d.%d.%d", a, b, c, d);
	*port = pa * 256 + pb;

	return 0;
}


//Download Files
int ftp_download(int socketfd, char *url_path) {
	int ret, port, data_socketfd;
	char ip[64], cmd_send[1024];
    

	//Queries data address
	ret = ftp_enter_passive(socketfd, ip, &port);
	if(ret != 0)
	{
		perror("Could not enter passive mode");
		socket_close(socketfd);
		return 1;
	}


	//Inits data socket connection
	ret = init(ip, port, &data_socketfd);
	if(ret != 0)
	{
	    perror("failed to init data port\r\n");
		socket_close(socketfd);
		return 1;
	}


	//Ready to download
	sprintf(cmd_send, "RETR %s\r\n", url_path);
	ret = ftp_send_command(socketfd, cmd_send);
	if(ret != 0)
	{
		perror("failed to send RETR command");
		socket_close(data_socketfd);
		socket_close(socketfd);
		return 1;
	}
	ret = ftp_rcv_response(socketfd);
	if(ret != RETRV_READY)
	{
		perror("failed to open binary mode connection");
		socket_close(data_socketfd);
		socket_close(socketfd);
		return 1;
	}


	//Downloads
	int file_fd, bytes_read;
	char url_path_copy[1024], buf[1024], *filename;
	strcpy(url_path_copy, url_path);

	char *token = strtok(url_path_copy, "/");
	while (token != NULL){
		filename = token;
		token = strtok(NULL, "/");
	}

	if ((file_fd = open(filename, O_WRONLY | O_CREAT, 0777)) < 0) {
		perror("failed to open file");
		socket_close(data_socketfd);
		socket_close(socketfd);
		return 1;
	}
	while((bytes_read = read(data_socketfd, buf, 1024)) > 0) {
		if (write(file_fd, buf, bytes_read) < 0){
			perror("Error while writing downloaded data to file");
			return 1;
		}
	}
	if (close(file_fd) < 0) {
		perror("Error closing file");
		return 1;
	}

	socket_close(data_socketfd);

	ret = ftp_rcv_response(socketfd);
	if (ret != FILE_READY) {
		perror("Couldn't retreive file");
		socket_close(socketfd);
		return 1;
	}

	
	//QUIT
	sprintf(cmd_send, "QUIT \r\n");
	ret = ftp_send_command(socketfd, cmd_send);
	if (ret != 0) {
		perror("Couldn't send QUIT command");
		socket_close(socketfd);
		return 1;
	}
	socket_close(socketfd);


	return 0;
}


int socket_close(int sockfd) {
	close(sockfd);
	return 0;
}



