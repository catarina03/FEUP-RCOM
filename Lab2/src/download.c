#include "download.h"



int url_parser(char *url, urlData *url_object){
    char *ftp = strtok(url, "/");
    char *args = strtok(NULL, "/");
    char *url_path = strtok(NULL, "");

    if (ftp == NULL) {
        perror("url should start with ftp");
        return -1;
    }

    if (args == NULL) {
        perror("No <host> or [<user>:<password>@]<host> declared");
        return -1;
    }

    if (url_path == NULL){
        perror("<url-path> is null");
        return -1;
    }

    char *user = strtok(strdup(args), ":");
    char *password  = strtok(NULL, "@");
    char *host = strtok(NULL, " ");

    if ((user != NULL) && (password != NULL)){
        strcpy(url_object->user, user);
        strcpy(url_object->password, password);
        strcpy(url_object->url_host, host);
    }
    else {
        user = "anonymous";
        password = "password";
        host = args;

        strcpy(url_object->user, user);
        strcpy(url_object->password, password);
        strcpy(url_object->url_host, host);
    }
    strcpy(url_object->url_path, url_path);

    printf("\n -- URL OBJECT -- \n");
    printf("User: %s\n", url_object->user);
    printf("Password: %s\n", url_object->password);
    printf("Host: %s\n", url_object->url_host);
    printf("URL path: %s\n", url_object->url_path);

    return 0;
}


int getIP(char host[], urlData *url_object) {
	struct hostent *h;

    /*
    struct hostent {
        char    *h_name;	Official name of the host. 
        char    **h_aliases;	A NULL-terminated array of alternate names for the host. 
        int     h_addrtype;	The type of address being returned; usually AF_INET.
        int     h_length;	The length of the address in bytes.
        char    **h_addr_list;	A zero-terminated array of network addresses for the host. 
        Host addresses are in Network Byte Order. 
    };
    #define h_addr h_addr_list[0]	The first address in h_addr_list. 
    */

    if ((h = gethostbyname(host)) == NULL) {  
        herror("gethostbyname");
        exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

    strcpy(url_object->host_name,  h->h_name);
    strcpy(url_object->ip, inet_ntoa(*((struct in_addr *)h->h_addr)));

    return 0;
}




int main(int argc, char *argv[])
{
    if (argc != 2) {
        perror("usage: download ftp://[<user>:<password>@]<host>/<url-path>");
        return 1;
    }

    urlData url_object;

    url_parser(argv[1], &url_object);
    getIP(url_object.url_host, &url_object);


    int socketfd, socketfd_rec;
    char command[256];
    char url_copy[256];
    strcpy(url_copy, argv[1]);

    // inits
    if (init(url_object.ip, 21, &socketfd) != 0){
        perror("Error: init()");
        return 1;
    }

    int response = ftp_rcv_response(socketfd);
    if(response != SERV_READY){
        perror("Received Bad Response");
        socket_close(socketfd);
        return 1;
    }


    // logs in
    if(ftp_login(socketfd, url_object.user, url_object.password) != 0){
        perror("Error logging in");
        return 1;
    }

    if(ftp_download(socketfd, url_object.url_path) != 0){
        perror("Error downloading file");
        return 1;
    }




    return 0;
}