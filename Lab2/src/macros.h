#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT_FTP 21
#define SERV_READY 220
#define USER_LOGIN 331
#define PASS_LOGIN 230
#define BIN_READY 200
#define PASV_READY 227
#define RETRV_READY 150
#define FILE_READY 226

