#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>

/* DNS .h files*/
#include "./starter_code/DNSHeader.h"
#include "./starter_code/DNSQuestion.h"
#include "./starter_code/DNSPack.h"

/* Command line arguments*/
static char *Log;
static int Listen_Port;
static int Geography_Based;
static char *Servers;
static FILE *Servers_File;
const char *Server_IP_List[100];

/* Socket Client(MiProxy) Setup */
int    sock_client, sock_new_client;
struct sockaddr_in sock_client_address;
int    sock_address_size = sizeof(sock_client_address);

/* Forking/Parallel */
int pid;

/* IP List */
int numIPs;
char * IPList [100];
char * PortList [100];

/* Methods */
void Usage (int argc, char *argv[]);
void Connect_MiProxy_To_DNS();
void Handle_Geography_Based();
void Handle_Round_Robin();
void Handle_Server_List(int type);

int main( int argc, char *argv[]){
	/* 1. Assign User Arguments */
	Usage(argc, argv);

	/* 2. Connect MiProxy to DNC*/
    Connect_MiProxy_To_DNS();

    /* 3. Use Round Robin (0) or Geography(1) Based Load Balancing*/
    if(Geography_Based == 0){
    	printf("Geography Based 0: %d \n", Geography_Based);
        Handle_Geography_Based();
    } else {
        printf("Round Robin 1: %d \n", Geography_Based);
        Handle_Round_Robin();
    }

	return 0;
}

void Usage (int argc, char *argv[]){
	if (argc < 5) {
		perror("Not enough arguments\n");
        exit(1);
	}
	Log = argv[1];
	sscanf(argv[2], "%d", &Listen_Port);
	sscanf(argv[3], "%d", &Geography_Based);
	Servers = argv[4];	
}

void Connect_MiProxy_To_DNS(){
	/* Socket Creation */
    sock_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_client < 0){
        perror("Client Connection: socket setup failed\n");
        exit(1);
    }

    /* Attaching socket to port */
    sock_client_address.sin_family = AF_INET;
    sock_client_address.sin_addr.s_addr = INADDR_ANY;/* INADDR_ANY; */
    sock_client_address.sin_port = htons( Listen_Port );

    /* Binding socket */
    if (bind(sock_client, (struct sockaddr *)&sock_client_address, sizeof(sock_client_address))<0) {
        perror("Client Socket Bind Failed");
        exit(1);
    }

    /* Listen for incoming clients */
    if (listen(sock_client, 8)<0) {
        perror("Client Socket Listen Failed");
        exit(1);
    } 

    /* miProxy will be accepting in an infinite while loop back in main*/
    printf("----------END OF Proxy -> DNS Setup----------\n");
}

void Handle_Geography_Based(){
    Handle_Server_List(0);
}

void Handle_Round_Robin(){
    Handle_Server_List(1);

    while(1){
        sock_new_client = accept(sock_client, (struct sockaddr *)&sock_client_address, (socklen_t*)&sock_address_size);
        if (sock_new_client<0) {
            perror("Accept Failed");
            exit(EXIT_FAILURE);
        } else {
            printf("NameServer socket for MiProxy: %d\n",sock_new_client );
        }

        pid = fork();
        if(pid < 0){
            perror("ERROR on fork");
            exit(1);
        } else if (pid == 0){

        } else {

        }
    }
}

void Handle_Server_List(int type){
    if(type == 0){
        /* Geography Based Txt file*/
    } else {
        /* Round Robin Based Txt file*/
        Servers_File = fopen(Servers, "r");
        char file_line[25];
        const char delimiter[2] = " ";
        char *token;

        while(fgets (file_line, 25, Servers_File)!= NULL){
            
            token = strtok(file_line, delimiter);
            IPList[numIPs] = token;
            token = strtok(NULL, delimiter);
            PortList[numIPs] = token;
            numIPs = numIPs + 1;
        }
    }
}