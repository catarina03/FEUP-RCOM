#include "macros.h"


typedef struct {
    char user[256];
    char password[256];
    char url_host[256];
    char url_path[256];
    char host_name[256];  // Host Name from gethostbyname()
    char ip[256];
} urlData;

int url_parser(char *url, urlData *url_object);

int getIP(char host[], urlData *url_object);