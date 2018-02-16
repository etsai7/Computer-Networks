
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>
#include <time.h>
#include <math.h>

static char *Model;
static int  Port;
static char *Host_IP;
static int  Time;

/* Socket Setup */
int sock, sock_new, valread;
struct sockaddr_in sock_address;
int sock_address_size = sizeof(sock_address);

/* Data between server and client */
char buffer[1000];
int data_rec = 0;
char data[1000] = {0};
int data_sent = 0;

/* Clock */
clock_t start, end, duration;

/* Get time of day*/
struct timeval t_start, t_end, temp;

void Usage( int argc, char *argv[] );
void Setup_Server();
void Setup_Client();

int main( int argc, char *argv[] ){
    Host_IP = "0.0.0.0";
    Model = "-c";
    Port = 4444;
    Time = 10;

    int i;
    for(i = 0; i < 1000; i++){
        data[i] = '0';
    }
    data[998] = '\n';
    data[999] = '\0';

    printf("\n------------------------------------------------------------------------------------------\n");
    Usage( argc, argv );

    if( !strncmp( Model, "-s", 2 )){
        printf("Starting Server\n");
        Setup_Server();
    } else if (!strncmp( Model, "-c", 2 )){
        printf("Starting Client\n");
        Setup_Client();
    }
}

void Usage(int argc, char *argv[]){

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
    printf("\n");
}

void Setup_Server(){
    /* Socket Creation */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Server: socket setup failed\n");
        exit(1);
    }
    printf("\nServer Socket: %d\n", sock);

    /* Attaching socket to port */
    sock_address.sin_family = AF_INET;
    sock_address.sin_addr.s_addr = inet_addr(Host_IP);/* INADDR_ANY; */
    sock_address.sin_port = htons( Port );

    /* Binding socket */
    if (bind(sock, (struct sockaddr *)&sock_address, sizeof(sock_address))<0)
    {
        perror("Bind Failed");
        exit(1);
    }

    /* Listen for incoming clients */
    if (listen(sock, 5)<0)
    {
        perror("Listen Failed");
        exit(1);
    } 
    
    /* Accepting 1st connection request */
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
    int begin = 1;

    while(1){
        ssize_t nb = recv( sock_new, &buffer, 1000, 0);
		if(begin == 1){
            gettimeofday(&t_start, NULL);
            begin = 0;
        }
        if (buffer[0] == 'b'){
            buffer[0] = 'a';
			printf("Sending ack packet\n");
            break;
        } 
        else if (nb != 0 || nb != -1){
			data_rec+= nb;
        }
		nb = 0;
    }
    fclose (file_fd);
    
    /* Statistics */
    gettimeofday(&t_end, NULL);
    send(sock_new , &buffer , sizeof(buffer) , 0 );

	printf("\nCLIENT STATISTICS\n");
    printf("Time Elapsed: %ld\n",(t_end.tv_sec - t_start.tv_sec));
    printf("Received: %d KB Rate=%f Mbps\n", data_rec/1000, (data_rec/1000*.008)/(t_end.tv_sec - t_start.tv_sec));
    printf("Data Received, Server Closing\n");
    close(sock_new);
    printf("------------------------------------------------------------------------------------------\n\n");
}

void Setup_Client(){
    /* Socket Creation */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        perror("Client: socket\n");
        exit(1);
    }
    printf("\nClient Socket: %d\n", sock);

    /* Set Target Server Address */
    memset(&sock_address, '0', sizeof(sock_address));
    sock_address.sin_family = AF_INET;
    sock_address.sin_port = htons( Port );
    if(inet_pton(AF_INET, Host_IP, &sock_address.sin_addr)<=0) /* Converts text to binary */
    {
        perror("Invalid address/ Address not supported \n");
        exit(1);
    }

    if(connect(sock, (struct sockaddr *)&sock_address, sizeof(sock_address)) < 0){
        perror("Fail to connect to server \n");
        exit(1);
    } 
   
    FILE * file_fd;
    file_fd = fopen("./test1.txt","w");
    /* Start time*/
    gettimeofday(&t_start, NULL);

    /* Send data*/
    while(1){
        gettimeofday(&temp,NULL);
        if((((temp.tv_sec*1000000 + temp.tv_usec) - (t_start.tv_sec * 1000000 + t_start.tv_usec))/(double)1000000) - Time > .000000000001){
            break;
        } else {
			ssize_t x =send(sock , &data , sizeof(data) , 0 );
			data_sent+=1;
        }
    }
    fclose(file_fd);

	printf("Client stopped sending\n");

    /* Sending out final closing package*/
	int j;
	for(j = 0; j < 999; j++){
		data[j] = 'b';
	}
    send(sock , &data , sizeof(data) , 0 );
	printf("Sending out final packet\n");
    
    /* Waiting for ack package*/
	recv( sock, &buffer, 1000,0);
	printf("Received final packet\n");

    /* Statistics*/
    gettimeofday(&t_end, NULL);

    printf("\nSERVER STATISTICS\n");
    printf("Time Elapsed: %ld\n",((t_end.tv_sec*1000000 + t_end.tv_usec) - (t_start.tv_sec * 1000000 + t_start.tv_usec))/ (long int)1000000);
    printf("Sent=%d KB Rate=%f Mbps\n", data_sent, (data_sent*.008)/(t_end.tv_sec - t_start.tv_sec));
    printf("Data Sent, Client Closing\n");
    close(sock);
    printf("------------------------------------------------------------------------------------------\n\n");
}
