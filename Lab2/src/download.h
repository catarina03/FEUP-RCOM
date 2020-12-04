#include "macros.h"


typedef struct {
    char *user;
    char *password;
    char *host;
    char *url_path;
} urlData;

int url_parser(char *url, urlData *url_object);