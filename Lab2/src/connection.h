#include "macros.h"


//int init(char *ip, int port, int *socketfd);
int init(char *ip, int port, int *socketfd);

int ftp_rcv_command(int socketfd);

int ftp_send_command(int sockfd, char *msg);

int ftp_login(int socketfd, char *username, char *password);

static int ftp_enter_passive(int socketfd, char *ip, int *port);

int ftp_download(int socketfd, char *url_path);

int socket_close(int sockfd);