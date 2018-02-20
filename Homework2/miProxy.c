#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

static char  *Log;
static float Alpha;
static int   Listen_Port;
static char  *DNS_IP;
static int   DNS_Port
static char  *www-ip;

/* Socket Setup */
int sock;
struct sockaddr_in sock_address;
