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
static int  Listen_Port;
static int  Geography_Based;
static char *Servers;
static FILE *Servers_File;


#define BUFSIZE 16033

/* Socket Client(MiProxy) Setup */
int    sock_client, sock_new_client;
struct sockaddr_in sock_client_address;
int    sock_address_size = sizeof(sock_client_address);

/* Forking/Parallel */
int pid;
int r_round;
int childret;

/* IP List */
int numIPs;
char  *Server_IP_List[100];
char IPList [100][40];
int  PortList [100];

/* DNS Data */
struct DNSPack     DP;
struct DNSHeader   DH;
struct DNSQuestion DQ;
struct DNSRecord   DR;

/* Methods */
void Usage (int argc, char *argv[]);
void Connect_MiProxy_To_DNS();
void Handle_Server_List(int type);
void Handle_Geography_Based();
int  Handle_Round_Robin();

int main( int argc, char *argv[]){
	/* 1. Assign User Arguments */
	Usage(argc, argv);

    /* 2. Use Round Robin (0) or Geography(1) Based Load Balancing*/
    if(Geography_Based == 1){
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

void Handle_Geography_Based(){
    Handle_Server_List(0);
}

int Handle_Round_Robin(){
    Handle_Server_List(0);

    /* Starting round is 0 */
    r_round = 0;
    
    int sockfd;                     /* socket */
    int portno;                     /* port to listen on */
    int clientlen;                  /* byte size of client's address */
    struct sockaddr_in serveraddr;  /* server's addr */
    struct sockaddr_in clientaddr;  /* client addr */
    struct hostent *hostp;          /* client host info */
    char buf[BUFSIZE];              /* message buf */
    char *hostaddrp;                /* dotted decimal host addr string */
    int optval;                     /* flag value for setsockopt */
    int n;                          /* message byte size */

    portno = Listen_Port;
    
    /* socket: create the parent socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets 
    * us rerun the server immediately after we kill it; 
    * otherwise we have to wait about 20 secs. 
    * Eliminates "ERROR on binding: Address already in use" error. 
    */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
             (const void *)&optval , sizeof(int));

    /* Build the server's address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /* bind: associate the parent socket with a port */
    if (bind(sockfd, (struct sockaddr *) &serveraddr, 
           sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

    clientlen = sizeof(clientaddr);

    while (1) {
        struct DNSPack DP_ret;
        memset(buf,'\0', BUFSIZE);

        n = recvfrom(sockfd, (char*)&buf, sizeof(struct DNSPack), 0,
                     (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0)
          error("ERROR in recvfrom");
      
        memcpy(&DP_ret, buf, n);
        DP_ret.DHeader.ID = DP_ret.DHeader.ID % numIPs;
        printf("server received %zu/%d round: %d IP: %s from %s\n", strlen(buf), n, r_round, IPList[r_round], DP_ret.DQuestion.QNAME);

        memset(DP_ret.DRecord.RDATA,'\0', sizeof(DP_ret.DRecord.RDATA));
        strncpy(DP_ret.DRecord.RDATA, Server_IP_List[r_round], strlen(Server_IP_List[r_round]));

        n = sendto(sockfd, (char*)&DP_ret, sizeof(struct DNSPack), 0, 
                   (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) 
          error("ERROR in sendto");
        r_round = (r_round + 1) % numIPs;
    }
}

void Handle_Server_List(int type){
    if(type == 1){
        /* Geography Based Txt file*/
    } else {
        /* Round Robin Based Txt file*/
        char *token;
        size_t bytes = 25;
        const char delimiter[2] = "\n";
        char * file_line = malloc(25 * sizeof(char));

        Servers_File = fopen(Servers, "r");\
        while(getline(&file_line, &bytes, Servers_File)!= -1){
            char * w = malloc(25 * sizeof(char));
            token = strtok(file_line, delimiter);
            strncpy(w, token, strlen(token));
            Server_IP_List[numIPs] = w;
            numIPs = numIPs + 1;
        }
        int j;
        printf("IPs: \n");
        for(j = 0; j < numIPs; j++){
            printf("\t%s\n", Server_IP_List[j]);
        }
        printf("lines: %d\n", numIPs);
    }
}
