#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libxml/xmlreader.h>

#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <sys/types.h>

#include <errno.h>

#define MAX_BUFFER 16033

static char  *Log;
static float Alpha;
static int   Listen_Port;
static char  *DNS_IP;
static int   DNS_Port;
static char  *www_ip;
static float T_cur;
static float T_new;

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
char buffer[MAX_BUFFER], method[300], file_location[300], http_version[300];
int bitrates[4];
ssize_t nb;
ssize_t y = MAX_BUFFER;

/* ----------Methods---------- */

    /* Setup */
void Usage (int argc, char *argv[]);
void Connect_ClientBrowser_To_MiProxy();
void Connect_MiProxy_To_Apache();

    /* Send and Receive */
void MiProxy_to_Server();
int  Server_to_MiProxy();
void MiProxy_to_Browser();

    /* f4m file */
int  Check_If_f4m_File();
void Handle_f4m_file();

    /* Video Chunks */
int  Check_If_Vid_Segments();
void Parse_Bit_Rates(char * xmlData);

    /* Initial Files*/
void Send_Initial_Files();
/* --------------------------- */

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
    int i;
    while(1)/*for(i = 0; i < 1; i++)*/{
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
        nb = recv( sock_new_client, &buffer, MAX_BUFFER, 0);
        printf("Data Received: %s\nof size %lu\n", buffer, nb);

        /* Split "GET /StrobeMediaPlayback.swf HTTP/1.1" to three char arrays*/
        sscanf(buffer,"%s %s %s",method,file_location,http_version);
        if(Check_If_f4m_File()){
            printf("Found an f4m file Request\n");
            Handle_f4m_file();
        } else if (Check_If_Vid_Segments()){
            printf("Found a video file Request\n");
            return 0;
            MiProxy_to_Server();
            Server_to_MiProxy();

        } else {
            printf("Found a initial file Request\n");
            Send_Initial_Files();
        }


        /* Use strlen to find actual length, not size of string*/
        /*printf("Method:        %s size: %lu\n", method, strlen(method));
        printf("File Location: %s size: %lu\n", file_location, strlen(file_location));
        printf("HTTP Version:  %s size: %lu\n", http_version, strlen(http_version));
        */

        printf("\n----------Data Transmission Portions----------\n");
        /* Pass along browser request to server*/
        printf("\n---------- PART 2 MiProxy -> Apache Server----------\n");
        printf("1. Buffer Data:\n\t%s", buffer);
        ssize_t x = send(sock_server , &buffer , nb , 0 );
        printf("2. Passed along browser request to server: %lu bytes\n", x);
        
        int j = 0;
        
        while(y == MAX_BUFFER){
            printf("\n---------- PART 3 Apache Server -> MiProxy----------\n");
            /* Receive server material*/
            printf("3. Receiving server material\n");
            y = recv( sock_server, &buffer, MAX_BUFFER,0);
            printf("4. Received server material: %lu bytes\n", y);
            printf("4.a Buffer Data:\n\t%s", buffer);

            printf("---------- PART 3 MiProxy -> Browser----------\n");
            /* Send off server material to Browser Client */
            printf("5.Sending off server material to Browser Client\n");
            printf("5.a Buffer Data:\n\t%s\n", buffer);
            ssize_t z = send(sock_new_client , &buffer , y , 0 );
            printf("6.Sent off server material to Browser Client: %lu bytes on Browser Socket: %d\n",z, sock_new_client);
            j++;
        }
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

/* 
* Connects Client to MiProxy. Acts as a Server
*/
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

/* 
* Connects MiProxy to Server
*/
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

/* 
* Transmits data from MiProxy to Apache server
*/
void MiProxy_to_Server(){
    printf("\n---------- PART 2 MiProxy -> Apache Server----------\n");
    printf("1. Buffer Data:\n%s", buffer);
    ssize_t x = send(sock_server , &buffer , nb , 0 );
    printf("2. Passed along browser request to server: %lu bytes\n", x);
    
}

/* 
* Transmits data from Apache server to MiProxy
* @return size in bytes received from server
*/
int Server_to_MiProxy(){
    printf("\n---------- PART 3 Apache Server -> MiProxy----------\n");
    /* Receive server material*/
    printf("3. Receiving server material\n");
    y = recv( sock_server, &buffer, MAX_BUFFER,0);
    printf("4. Received server material: %lu bytes\n", y);
   /* printf("4.a Buffer Data:\n\t%s", buffer);*/
    return y;
}

/* 
* Transmits data from MiProxy to Browser
*/
void MiProxy_to_Browser(){
    printf("---------- PART 3 MiProxy -> Browser----------\n");
    /* Send off server material to Browser Client */
    printf("5.Sending f4m File to Browser Client\n");
    printf("5.a Buffer Data:\n\t%s\n", buffer);
    ssize_t z = send(sock_new_client , &buffer , y , 0 );
    printf("6.Sent off server material to Browser Client: %lu bytes on Browser Socket: %d\n",z, sock_new_client);
}

/*
* Checks if the file requested  is a f4m file
* @return 1 if found 0 if not
*/
int Check_If_f4m_File(){
    char *found = NULL;
    found =  strstr(file_location,".f4m");

    if(found == NULL){
        return 0;
    }
    return 1;
}

/*
* Handles the parsing and transmission of f4m (xml) file
*/
void Handle_f4m_file(){
    char f4mGetbuffer[MAX_BUFFER];
    memcpy(f4mGetbuffer, buffer, sizeof(buffer));
    /*printf("f4m Buffer:\n%s",f4mGetbuffer);*/

    MiProxy_to_Server();
    Server_to_MiProxy();

    /* Look for the xml file data from response*/
    char *data;
    data = strstr(buffer, "<?xml");

    Parse_Bit_Rates(data);

    int j;
    for(j=0;j<4;j++){
        printf("Bitrate: %d\n", bitrates[j]);
    }

    /* Editing the request to have no bitrates f4m*/
    char file_location_substring[300];
    memset(&buffer[0], 0, sizeof(buffer));
    strcpy(buffer, method);
    strcat(buffer, " ");
    memcpy(file_location_substring, file_location, strlen(file_location)-4);
    strcat(buffer, file_location_substring);
    strcat(buffer, "_nolist");
    strcat(buffer, strstr(f4mGetbuffer,".f4m"));
    printf("\nTrying to build up new Buffer: \n%s of size: %ld\n",buffer, strlen(buffer));

    nb = strlen(buffer);
    MiProxy_to_Server();
    Server_to_MiProxy();

    MiProxy_to_Browser();
}

/*
* Parses the xml file data for bitrates and stores in int bitrates []
* @params xmlData the pointer to xml data from server response
*/
void Parse_Bit_Rates(char * xmlData){
    /* Maybe should copy buffer instead of writing to file*/

    FILE *file = fopen("f4mfile.txt", "w");
    FILE *file2;
    char * line = NULL;
    ssize_t read;
    size_t len = 0;
    int i = 0;

    int results = fputs(xmlData, file);
    fclose(file);

    file = fopen("f4mfile.txt", "r");


    if (file) {
        /* Read file line by line*/
        while ((read = getline(&line, &len, file)) != -1) {
            char*token;
            long value;
            char*ptr;
            char str2[5];

            if(strstr(line,"bitrate")){
                printf("%s", line);
                token = strtok(line, "=");
                while( token != NULL ) {
                    /*printf("Token is: %s\n", token);*/

                    strncpy ( str2, token+1, strlen(token) - 2 );
                    /*printf("str2: %s\n",str2);*/
                    value = strtol(str2, &ptr,10);
                    token = strtok(NULL, "=");
                }
                printf("Value Parsed: %ld\n", value);
                bitrates[i] = value;
                i++;
            }
        }
    }
    fclose(file);
    remove("f4mfile.txt");
}

int Check_If_Vid_Segments(){
    char *found = NULL;
    found =  strstr(file_location,"Seg");

    if(found == NULL){
        return 0;
    }
    return 1;
/*T_cur = Alpha * T_new + (1 - Alpha) * T_cur;*/
}


void Send_Initial_Files(){

}
/* Use Select on miProxy */
