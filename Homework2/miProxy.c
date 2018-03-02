#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libxml/xmlreader.h>

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

static int Server_Port = 80;

/* Socket Client Setup */
int    sock_client, sock_new_client;
struct sockaddr_in sock_client_address;
int    sock_address_size = sizeof(sock_client_address);

/* Socket Server Setup */ 
int    sock_server, sock_new_server;
struct sockaddr_in sock_server_address;
int    sock_address_server_size = sizeof(sock_server_address);

/* Data */
char buffer[1000], method[300], file_location[300], http_version[300];


/* Methods */
void Usage (int argc, char *argv[]);
void Connect_Client();
void Connect_Server();

/* ./miProxy test.txt .5 1025 0.0.0.0 2555 */
int main( int argc, char *argv[] ){
	www_ip = "video.cse.umich.edu";

    /* 1. Assign User Arguments */
	Usage(argc, argv);

    /* 2. Connect Browser Client to miProxy*/
    Connect_Client();

    /* 3. Connect miProxy to Apache Server */
    Connect_Server();

    /* 4. Received GET requests from Browser Client*/
    while(1){
        /* Clear out the buffer and the separate char arrays */
        memset(&buffer[0], 0, sizeof(buffer));
        memset(&method[0], 0, sizeof(method));
        memset(&file_location[0], 0, sizeof(file_location));
        memset(&http_version[0], 0, sizeof(http_version));


        ssize_t nb = recv( sock_new_client, &buffer, 1000, 0);
        printf("\nData Received: %s of size %lu\n ", buffer, nb);

        sscanf(buffer,"%s %s %s",method,file_location,http_version);

        /* Use strlen to find actual length, not size of string*/
        printf("Method:        %s size: %lu\n", method, strlen(method));
        printf("File Location: %s size: %lu\n", file_location, strlen(file_location));
        printf("HTTP Version:  %s size: %lu\n", http_version, strlen(http_version));
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

/* Connects Client to MiProxy*/
void Connect_Client(){
/* Socket Creation */
    sock_client = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_client < 0){
        perror("Client Connection: socket setup failed\n");
        exit(1);
    }

    printf("\nMiProxy socket to accept from Browser: %d\n", sock_client);

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

/* Connects MiProxy to Server*/
void Connect_Server(){
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server < 0){
        perror("Server: socket setup failed\n");
        exit(1);
    }
    printf("\nMiProxy socket to connect to Apache Server: %d\n", sock_server);

    /* Set Target Server Address */
    memset(&sock_server_address, '0', sizeof(sock_server_address));
    sock_server_address.sin_family = AF_INET;
    sock_server_address.sin_port = htons( Server_Port );
    if(inet_pton(AF_INET, www_ip, &sock_server_address.sin_addr)<=0) /* Converts text to binary */
    {
        perror("Invalid address/ Address not supported \n");
        exit(1);
    }

    if(connect(sock_server, (struct sockaddr *)&sock_server_address, sizeof(sock_server_address)) < 0){
        perror("Fail to connect to server \n");
        exit(1);
    }
}

// Use Select on miProxy
