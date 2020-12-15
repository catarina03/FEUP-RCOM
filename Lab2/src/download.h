#include "macros.h"


typedef struct {
    char user[256];
    char password[256];
    char host[256];
    char url_path[256];
} urlData;

int url_parser(char *url, urlData *url_object);