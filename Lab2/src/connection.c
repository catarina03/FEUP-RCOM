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


int read_socket(int socketfd){
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

/*
//Logon Server
int ftp_login(char *addr, int port, char *username, char *password)
{
	int ret;
	LOG_INFO("connect...\r\n");
	ret = socket_connect(m_socket_cmd, addr, port);
	if(ret != 1)
	{
		LOG_INFO("connect server failed!\r\n");
		return 0;
	}
	LOG_INFO("connect ok.\r\n");
    //Waiting for Welcome Message
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 220)
	{
		LOG_INFO("bad server, ret=%d!\r\n", ret);
		socket_close(m_socket_cmd);
		return 0;
	}
	
	LOG_INFO("login...\r\n");
    //Send USER
	sprintf(m_send_buffer, "USER %s\r\n", username);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 1)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 331)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	
    //Send PASS
	sprintf(m_send_buffer, "PASS %s\r\n", password);
	ret = ftp_send_command(m_send_buffer);
	if(ret != 1)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 230)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	LOG_INFO("login success.\r\n");
	
    //Set to binary mode
	ret = ftp_send_command("TYPE I\r\n");
	if(ret != 1)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	ret = ftp_recv_respond(m_recv_buffer, 1024);
	if(ret != 200)
	{
		socket_close(m_socket_cmd);
		return 0;
	}
	return 1;
}
*/


int close(sockfd) {
	close(sockfd);
	return 0;
}



