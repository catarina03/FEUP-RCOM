#include "macros.h"


int init(char *ip, int port, int *socketfd);

int ftp_rcv_command(int socketfd);

int ftp_send_command(int sockfd, char *msg);

int ftp_login(int socketfd, char *username, char *password);

int socket_close(int sockfd);