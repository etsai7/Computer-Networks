#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>

static char  *Log;
static float Alpha;
static int   Listen_Port;
static char  *DNS_IP;
static int   DNS_Port;
static char  *www_ip;

/* Socket Setup */
int sock_client, sock_new_client;
struct sockaddr_in sock_client_address;
int sock_address_size = sizeof(sock_client_address);

/* Data */
char buffer[1000];

/* Methods */
void Usage (int argc, char *argv[]);
void Connect_Client();
void Connect_Server();

/* ./miProxy test.txt .5 1025 0.0.0.0 2555 */
int main( int argc, char *argv[] ){
	www_ip = "video.cse.umich.edu";

	Usage(argc, argv);

    Connect_Client();

    while(1){
        ssize_t nb = recv( sock_new_client, &buffer, 1000, 0);
        printf("\nData Received: %s of size %lu\n ", buffer, nb);
    }

	return 0;
}

void Usage(int argc, char *argv[]){
	if (argc < 6) {
		perror("Not enough arguments\n");
        exit(1);
	}
	Log = argv[1];
	sscanf(argv[2], "%f", &Alpha);
	sscanf(argv[3], "%d", &Listen_Port);
	DNS_IP = argv[4];
	sscanf(argv[5], "%d", &DNS_Port);
	if(argc == 7){
		www_ip = argv[6];
	}

	printf("\n--------------------MiProxy Information--------------------\n");
	printf("Log Path:     %s\n", Log);
	printf("Alpha:        %f\n", Alpha);
	printf("Listen Port:  %d\n", Listen_Port);
	printf("DNS IP:       %s\n", DNS_IP);
	printf("DNS Port:     %d\n", DNS_Port);
	printf("www-ip:       %s\n", www_ip);
	printf("-----------------------------------------------------------\n\n ");
}

void Connect_Client(){
/* Socket Creation */
    sock_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_client < 0){
        perror("Client Connection: socket setup failed\n");
        exit(1);
    }

    printf("\nMiProxy socket to accept: %d\n", sock_client);

    /* Attaching socket to port */
    sock_client_address.sin_family = AF_INET;
    sock_client_address.sin_addr.s_addr = INADDR_ANY;/* INADDR_ANY; */
    sock_client_address.sin_port = htons( Listen_Port );

    /* Binding socket */
    if (bind(sock_client, (struct sockaddr *)&sock_client_address, sizeof(sock_client_address))<0)
    {
        perror("Client Socket Bind Failed");
        exit(1);
    }

    /* Listen for incoming clients */
    if (listen(sock_client, 8)<0)
    {
        perror("Client Socket Listen Failed");
        exit(1);
    } 

    sock_new_client = accept(sock_client, (struct sockaddr *)&sock_client_address, (socklen_t*)&sock_address_size);
    if (sock_new_client<0)
    {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    } else {
        printf("MiProxy socket for client: %d\n",sock_new_client );
    }

    printf("At the end of the client -> proxy setup\n");
}

