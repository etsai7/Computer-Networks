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
const char  *Server_IP_List[100];

#define BUFSIZE 16000

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
void Recv_DNSPack();
void Send_DNSPack();

int main( int argc, char *argv[]){
	/* 1. Assign User Arguments */
	Usage(argc, argv);

	/* 2. Connect MiProxy to DNC*/
    /*Connect_MiProxy_To_DNS();*/

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

int Handle_Round_Robin(){
    Handle_Server_List(1);

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

    /* 
    * bind: associate the parent socket with a port 
    */
    if (bind(sockfd, (struct sockaddr *) &serveraddr, 
           sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

    /* 
    * main loop: wait for a datagram, then echo it
    */
    clientlen = sizeof(clientaddr);
    while (1) {
    struct DNSPack DP_ret;
    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, (char*)&buf, sizeof(struct DNSPack), 0,
                 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");
    memcpy(&DP_ret, buf, n);
    DP_ret.DHeader.ID = DP_ret.DHeader.ID % numIPs;
    printf("server received %zu/%d bytes: %d\n", strlen(buf), n, DP_ret.DHeader.ID);

    /* 
     * sendto: echo the input back to the client 
     */
    n = sendto(sockfd, (char*)&DP_ret, sizeof(struct DNSPack), 0, 
               (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
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
        numIPs = 0;
        while(fgets (file_line, 25, Servers_File)!= NULL){
            
            strncpy(IPList[numIPs], file_line, strlen(file_line)-1);
            printf("Copying: %s\n", IPList[numIPs]);
            numIPs = numIPs + 1;
        }
        printf("IPs:\n %s \n %s \n %s \n", IPList[0], IPList[1], IPList[2]);
        printf("lines: %d\n", numIPs);
        fflush(stdout);
    }
}

void Recv_DNSPack(){
    printf("Receiving DNS Pack \n");
    char recvd[sizeof(struct DNSPack)];
    ssize_t nb = recv( sock_new_client, (char*)&recvd, sizeof(struct DNSPack), 0 );
    memcpy(&DP, recvd, nb);
    DH = DP.DHeader;
    DQ = DP.DQuestion;
    DR = DP.DRecord;

    printf("DH Id: %d\n", DH.ID);
    printf("DQ QName: %s\n", DQ.QNAME);
    printf("DR Type: %d\n", DR.TYPE);
}

void Send_DNSPack(){
    printf("Sending DNS Pack \n");
}