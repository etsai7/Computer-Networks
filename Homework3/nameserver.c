#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>

static char *Log;
static int Listen_Port;
static int Geography_Based;
static char *Servers;
static FILE *Serverse_File;
const char *Server_IP_List[10];

void Usage (int argc, char *argv[]);

int main( int argc, char *argv[]){
	/* 1. Assign User Arguments */
	Usage(argc, argv);

	return 0;
}

void Usage (int argc, char *argv[]){
	if (argc < 6) {
		perror("Not enough arguments\n");
        exit(1);
	}
	Log = argv[1];
	sscanf(argv[2], "%d", &Listen_Port);
	sscanf(argv[3], "%d", &Geography_Based);
	Servers = argv[4];
	
}