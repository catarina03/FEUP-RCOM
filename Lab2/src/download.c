#include "download.h"



int url_parser(char *url, urlData *url_object){
    char *ftp = strtok(url, "/");
    char *args = strtok(NULL, "/");
    char *url_path = strtok(NULL, "/");

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
        url_object->user = user;
        url_object->password = password;
        url_object->host = host;
    }
    else {
        url_object->user = "anonymous";
        url_object->password = "password";
        url_object->host = args;
    }
    url_object->url_path = url_path;

    printf("User: %s\n", url_object->user);
    printf("Password: %s\n", url_object->password);
    printf("Host: %s\n", url_object->host);
    printf("URL path: %s\n", url_object->url_path);

    return 0;
}




int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    urlData url_object;

    url_parser(argv[1], &url_object);

    return 0;
}