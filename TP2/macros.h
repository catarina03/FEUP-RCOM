#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>


#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


#define FLAG 0x7E
#define A 0x03
#define SET 0x03
#define UA 0x07
#define SET_BCC A ^ SET
#define UA_BCC A ^ UA