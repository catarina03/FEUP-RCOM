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
#define MAX_SIZE 100

#define FLAG 0x7E
#define A 0x03
#define SET 0x03
#define UA 0x07
#define SET_BCC A ^ SET
#define UA_BCC A ^ UA

#define ESC 0x7d
#define ESC_ESC 0x5d
#define ESC_FLAG 0x5e

#define CONTROL_I(r) ((r == 0) ? 0x00 : 0x40)

#define TRANSMITTER 1234
#define RECEIVER 4321