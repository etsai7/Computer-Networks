
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>
#include <time.h>

static char *Model;
static int  Port;
static char *Host_IP;
static int  Time;

// Socket Setup
int sock, sock_new, valread;
struct sockaddr_in sock_address;
int sock_address_size = sizeof(sock_address);

// Data between server and client
char buffer[1000];
char *hello = "Hello from server";
char *hello_client = "Hello from client";
char buffer_client[1000] = {0};
char data[1000] = {0};

// Clock
clock_t start, end;

void Usage( int argc, char *argv[] );
void Setup_Server();
void Setup_Client();

int main( int argc, char *argv[] ){
    //Host_IP = "127.0.0.1";
    Host_IP = "0.0.0.0";
    Model = "-c";
    Port = 4444;
    Time = 10;

    int i;
    for(i = 0; i < 1000; i++){
        data[i] = '0';
    }
    data[999] = '\n';

    Usage( argc, argv );

    if( !strncmp( Model, "-s", 2 )){
        printf("Starting server\n");
        Setup_Server();
    } else if (!strncmp( Model, "-c", 2 )){
        printf("Starting client\n");
        Setup_Client();
    }
}

void Usage(int argc, char *argv[]){

    printf("\n");
    while(--argc > 0 ){
        argv++;

        if( !strncmp( *argv, "-p", 2 ) ) {  /*Strncmp is string compare. Returns 0 if equal.*/
            sscanf(argv[1], "%d", &Port );  /*The [1] is always the next one from the current pointer. So if we were at [2]. argv[1] is technically [3] */
            printf("Port Number: %d\n", Port);
            argc--; argv++;
        } else if( !strncmp( *argv, "-h", 2 )) {
            Host_IP = argv[1];
            printf("Host IP:     %s\n", Host_IP);
            argc--; argv++;
        } else if(!strncmp( *argv, "-t", 2 )) {
            sscanf(argv[1], "%d", &Time );
            printf("Time:        %d\n", Time);
            argc--; argv++;
        } else if(!strncmp( *argv, "-c", 2) || !strncmp( *argv, "-s", 2)){
            Model = argv[0];
            printf("Model:       %s\n", Model);
            argc--;
        }
    }
    // int sr = socket(AF_INET, SOCK_DGRAM, 0);
    // printf("\nSocket: %d\n", sr);
    printf("\n");
}

void Setup_Server(){
    // Socket Creation
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Server: socket setup failed\n");
        exit(1);
    }
    printf("\nServer Socket: %d\n", sock);

    // Attaching socket to port
    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = inet_addr(Host_IP);//INADDR_ANY;
    sock_address.sin_port = htons( Port );

    // Binding socket
    if (bind(sock, (struct sockaddr *)&sock_address, sizeof(sock_address))<0)
    {
        perror("Bind Failed");
        exit(1);
    }

    // Listen for incoming clients
    if (listen(sock, 5)<0)
    {
        perror("Listen Failed");
        exit(1);
    } 
    
    // Accepting 1st connection request
    sock_new = accept(sock, (struct sockaddr *)&sock_address, (socklen_t*)&sock_address_size);
    if (sock_new<0)
    {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    } else {
        printf("Accepted Client\n");
    }

    FILE * file_fd;
    file_fd = fopen ("./test.txt","w");
    start = clock();

    while(1){
        ssize_t nb = recv( sock_new, &buffer, 1000, 0);
        if ( nb == -1 ) err( "recv failed" );
        else if ( nb == 0 ) printf("nb = 0\n"); /* got end-of-stream */
        else if (buffer[0] == 'b') break;
        else fwrite(buffer, sizeof(buffer),1,file_fd);
    }
    fclose (file_fd);

    // valread = read( sock_new , buffer, 1000);
    printf("%s\n",buffer);
    //send(sock_new , hello , strlen(hello) , 0 );
    printf("Hello message sent\n");
}

void Setup_Client(){
    // Socket Creation
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Client: socket\n");
        exit(1);
    }
    printf("\nClient Socket: %d\n", sock);

    // Set Target Server Address
    memset(&sock_address, '0', sizeof(sock_address));
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons( Port );
    if(inet_pton(AF_INET, Host_IP, &sock_address.sin_addr)<=0) // Converts text to binary
    {
        perror("Invalid address/ Address not supported \n");
        exit(1);
    }

    if(connect(sock, (struct sockaddr *)&sock_address, sizeof(sock_address)) < 0){
        perror("Fail to connect to server \n");
        exit(1);
    } 

    start = clock();
    while((double) (clock() - start) / CLOCKS_PER_SEC < Time){
        send(sock , &data , sizeof(data) , 0 );
    }
    printf("Time Elapsed: %f\n",(double) (clock() - start) / CLOCKS_PER_SEC);

    data[0] = 'b';
    send(sock , &data , sizeof(data) , 0 );

/*
    send(sock , hello_client , strlen(hello_client) , 0 );
    printf("Hello message sent\n");
    valread = read( sock , buffer_client, 1000);
    printf("%s\n",buffer_client );
    */
}