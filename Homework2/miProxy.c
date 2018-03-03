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
char buffer[16000], method[300], file_location[300], http_version[300];


/* Methods */
void Usage (int argc, char *argv[]);
void Connect_ClientBrowser_To_MiProxy();
void Connect_MiProxy_To_Apache();

/* ./miProxy test.txt .5 1025 0.0.0.0 2555 */
int main( int argc, char *argv[] ){
	www_ip = "video.cse.umich.edu";

    /* 1. Assign User Arguments */
	Usage(argc, argv);

    /* 2. Connect Browser Client to miProxy*/
    Connect_ClientBrowser_To_MiProxy();

    /* 3. Connect miProxy to Apache Server */
    Connect_MiProxy_To_Apache();

    /* 4. Received GET requests from Browser Client*/
    while(1){
        /* Clear out the buffer and the separate char arrays */
        printf("Transmitting Data\n");

        memset(&buffer[0], 0, sizeof(buffer));
        memset(&method[0], 0, sizeof(method));
        memset(&file_location[0], 0, sizeof(file_location));
        memset(&http_version[0], 0, sizeof(http_version));

        /*GET Request from Browser: 
            Data Received: GET / HTTP/1.1
            Host: 10.0.0.2:1025
            User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0
            Accept: text/html,application/xhtml+xml,application/xml;q=0.9,asterisk(*)/*;q=0.8
            Accept-Language: en-US,en;q=0.5
            Accept-Encoding: gzip, deflate
            Connection: keep-alive
        */
        printf("\n---------- PART 1 Browser -> MiProxy----------\n");
        ssize_t nb = recv( sock_new_client, &buffer, 16000, 0);
        printf("Data Received: %sof size %lu\n", buffer, nb);

        sscanf(buffer,"%s %s %s",method,file_location,http_version);

        /* Use strlen to find actual length, not size of string*/
        printf("Method:        %s size: %lu\n", method, strlen(method));
        printf("File Location: %s size: %lu\n", file_location, strlen(file_location));
        printf("HTTP Version:  %s size: %lu\n", http_version, strlen(http_version));

        printf("\n----------Data Transmission Portions----------\n");
        /* Pass along browser request to server*/
        printf("\n---------- PART 2 MiProxy -> Apache Server----------\n");
        printf("1. Buffer Data:\n\t%s", buffer);
        ssize_t x = send(sock_server , &buffer , nb , 0 );
        printf("2. Passed along browser request to server: %lu bytes\n", x);
        
        printf("\n---------- PART 3 Apache Server -> MiProxy----------\n");
        /* Receive server material*/
        printf("3. Receiving server material\n");
        ssize_t y = recv( sock_server, &buffer, 16000,0);
        printf("4. Received server material: %lu bytes\n", y);
        printf("4.a Buffer Data:\n\t%s", buffer);

        printf("---------- PART 3 MiProxy -> Browser----------\n");
        /* Send off server material to Browser Client */
        printf("5.Sending off server material to Browser Client\n");
        printf("5.a Buffer Data:\n\t%s", buffer);
        ssize_t z = send(sock_new_client , &buffer , y , 0 );
        printf("6.Sent off server material to Browser Client: %lu bytes on Browser Socket: %d\n",z, sock_new_client);
    
        /* Temporary termination return*/
        /* return 0; */
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

/* Connects Client to MiProxy. Acts as a Server*/
void Connect_ClientBrowser_To_MiProxy(){
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

    printf("----------END OF Browser Client -> Proxy Setup----------\n");
}

/* Connects MiProxy to Server*/
void Connect_MiProxy_To_Apache(){
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

    printf("----------END OF Proxy -> Apache Setup----------\n");
}

/* Use Select on miProxy */
