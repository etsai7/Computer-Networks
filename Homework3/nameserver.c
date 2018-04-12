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
#define BUFFER_SIZE 160333
#define IN 99

/* Socket Client(MiProxy) Setup */  
int sockfd;                     /* socket */
int clientlen;                  /* byte size of client's address */
struct sockaddr_in serveraddr;  /* server's addr */
struct sockaddr_in clientaddr;  /* client addr */
struct hostent *hostp;          /* client host info */
char buf[BUFSIZE];              /* message buf */
char *hostaddrp;                /* dotted decimal host addr string */
int optval;                     /* flag value for setsockopt */
int n;                          /* message byte size */

/* Forking/Parallel */
int pid;
int r_round;
int childret;

/* IP List */
int  numIPs;
char *Server_IP_List[100];
char IPList [100][40];
int  PortList [100];

/* Network Data*/
char Network_Info[100][100];
int  num_line_network;
char client_IP_str[100];
char dst_return_server_IP[100];
int  num_nodes = 1;
int  client_id, servers_id[50];
char servers_IP[50][100];   /*can store max. 50 servers; each has max 100 bits*/
int num_links, num_servers;
int src[100], dst[100], weight[100], cost[100][100]; 

/* DNS Data */
struct DNSPack     DP;
struct DNSHeader   DH;
struct DNSQuestion DQ;
struct DNSRecord   DR;

/* Methods */
void Usage (int argc, char *argv[]);
void Open_For_Connection();
void Handle_Server_List(int type);
void Handle_Geography_Based();
int  Handle_Round_Robin();

/* Distance Calculations*/
void Distance();
void Get_closest_server();

/* Helper Methods for Distance Calculations*/
void Get_num_nodes();
void Find_client_id();
void Find_servers_id();
void Get_num_links_weights();
void Create_cost_graph(); 
int  dijkstra(int source, int target);

int main( int argc, char *argv[]){
	/* 1. Assign User Arguments */
	Usage(argc, argv);

    /* 2. Open Port For UDP Requests*/
    Open_For_Connection();

    /* 3. Use Round Robin (0) or Geography(1) Based Load Balancing*/
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

void Open_For_Connection(){
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
    serveraddr.sin_port = htons((unsigned short)Listen_Port);

    /* bind: associate the parent socket with a port */
    if (bind(sockfd, (struct sockaddr *) &serveraddr, 
           sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

    clientlen = sizeof(clientaddr);
}

void Handle_Geography_Based(){
    Handle_Server_List(1);
    Distance();
}

int Handle_Round_Robin(){
    Handle_Server_List(0);

    /* Starting round is 0 */
    r_round = 0;

    while (1) {
        struct DNSPack DP_ret;
        memset(buf,'\0', BUFSIZE);

        n = recvfrom(sockfd, (char*)&buf, sizeof(struct DNSPack), 0, (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0)
          error("ERROR in recvfrom");
      
        memcpy(&DP_ret, buf, n);
        DP_ret.DHeader.ID = DP_ret.DHeader.ID % numIPs;
        printf("server received %zu/%d round: %d IP: %s from %s\n", strlen(buf), n, r_round, Server_IP_List[r_round], DP_ret.DQuestion.QNAME);

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
        printf("Geography Based Text File\n");
        ssize_t read;
        size_t len = 0;
        char * line = NULL;
        num_line_network = 0;

        Servers_File = fopen(Servers, "r");
        printf("Opened File\n");
        if(Servers_File){
            while((read = getline(&line, &len, Servers_File)) != -1){
                strncpy(Network_Info[num_line_network], line, strlen(line)-1);
                num_line_network++;
            }
        }
        fclose(Servers_File);

        int i = 0;
        printf("Network Info: \n");
        for(i = 0; i < num_line_network; i++){
            printf("\t%s\n", Network_Info[i]);
        }
        printf("lines: %d\n", num_line_network);

    } else {
        /* Round Robin Based Txt file*/
        printf("Round Robin Text File\n");
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
        fclose(Servers_File);

        int j;
        printf("IPs: \n");
        for(j = 0; j < numIPs; j++){
            printf("\t%s\n", Server_IP_List[j]);
        }
        printf("lines: %d\n", numIPs);
    }
}

void Distance(){
    while(1){
        struct DNSPack pack;
        memset(&buf[0], 0, sizeof(buf));

        n = recvfrom(sockfd, (char*)&buf, sizeof(struct DNSPack), 0, (struct sockaddr *)&clientaddr,  &clientlen);
        if(n < 0){
            perror("error in receiving request from miProxy\n");
            exit(1);
        }
        memcpy(&pack, buf, n);
        inet_ntop(AF_INET, &clientaddr.sin_addr, client_IP_str, 200); 
        printf("client IP is: %s\n", client_IP_str);

        /*find the apache server closest to miProxy */
        Get_closest_server();

        /* change info on the DNSPack Pack we got from miProxy*/
        memset(&pack.DRecord.RDATA[0], 0, sizeof(pack.DRecord.RDATA));
        printf("IP to send back: %s\n", dst_return_server_IP);
        strncpy(pack.DRecord.RDATA, dst_return_server_IP, sizeof(dst_return_server_IP));
        pack.DHeader.AA = 1;
        pack.DRecord.TYPE = 1;
        pack.DRecord.CLASS = 1;
        pack.DRecord.TTL = 0;
        pack.DRecord.RDLENGTH = sizeof(pack.DRecord.RDATA);

        /*send the updated Pack back to miProxy*/
        n = sendto(sockfd, (char*)&pack, sizeof(struct DNSPack), 0, (struct sockaddr *)&clientaddr, clientlen);
        if(n < 0) {
            perror("error in sending respone to miProxy\n");
            exit(1);
        } 
    }
}

void Get_closest_server(){
    printf("Getting Closest Server\n");
    Get_num_nodes();
    Find_client_id();
    Find_servers_id();
    Get_num_links_weights();
    Create_cost_graph();
}

void Get_num_nodes(){
    printf("\t-->Getting Number of Nodes\n");
    char *pch;
    char buf[100];
    memset(buf, '\0', 100);
    strncpy(buf, Network_Info[0], sizeof(Network_Info[0]));
    printf("\t\tbuf: %s\n", buf);
    pch = strtok(buf, "NUM_NODES: ");
    printf("\t\tpch: %s\n", pch);
    sscanf(pch, "%d", &num_nodes);
    printf("\t\tNumber of nodes : %d\n", num_nodes);  
}

void Find_client_id(){
    printf("\t-->Finding Client ID\n");
    char str[100];
    char *pch, *tok; 
    int i = 1;
    while(i < num_line_network){
        memset(&str[0], 0, sizeof(str));
        strncpy(str, Network_Info[i], sizeof(Network_Info[i]));
        printf("\t\tAfter string copy, str is: %s\n", str);
        tok = strstr(str, client_IP_str);
        if(tok != NULL){
            pch = strtok(str, " CLIENT ");
            printf("\t\tpch: %s\n", pch);
            sscanf(pch, "%d", &client_id);
            printf("\t\tClient id is : %d\n", client_id);
            break;
        }
        i++;
    }
}

void Find_servers_id(){
    printf("\t--> Finding Server ID\n");
    char str[100];
    char *pch, *tok; 
    int i = 1;
    int num_s = 0;
    int nLine = num_line_network; /*no need to check first line*/
    while(i < nLine){
        memset(&str[0], 0, sizeof(str));
        strncpy(str, Network_Info[i], sizeof(Network_Info[i]));
        printf("\t\tAfter string copy, str is: %s\n", str);
        tok = strstr(str, " SERVER ");
        if(tok != NULL){
            pch = strtok(str, " SERVER ");
            printf("\t\tpch: %s\n", pch);
            sscanf(pch, "%d", &servers_id[num_s]);
            pch = strtok(NULL, " SERVER ");
            strncpy(servers_IP[num_s], pch, sizeof(pch));
            printf("\t\tserver id at %d is: %d\n", num_s, servers_id[num_s]);
            printf("\t\tserver IP at %d is: %s\n", num_s, servers_IP[num_s]);
            num_s ++;
        }
        i ++;
    }
    numIPs = num_s;
}

void Get_num_links_weights(){
    printf("\t-->Getting Number of Link Weights\n");
    char str[100];
    char *pch, *tok;
    int i = 1, j = 0;
    while(i < num_line_network){
        memset(&str[0], 0, sizeof(str));
        strncpy(str, Network_Info[i], sizeof(Network_Info[i]));
        printf("\t\tAfter string copy, str is: %s\n", str);
        tok = strstr(str, "NUM_LINKS: ");
        if(tok != NULL){
            pch = strtok(str, "NUM_LINKS: ");
            printf("\t\tpch: %s\n", pch);
            sscanf(pch, "%d", &num_links);
            printf("\t\tNumber of links is : %d\n", num_links);
            break;
        }
        i++;
    }
    printf("\t\tCreating weight graph\n");
    printf("\t\tNum line network: %d\n", num_line_network);
    i++;
    while(i < num_line_network){
        memset(&str[0], 0, sizeof(str));
        strncpy(str, Network_Info[i], sizeof(Network_Info[i]));
        printf("\t\t<origin_id> <destination_id> <cost>, str is: %s\n", str);
        pch = strtok(str, " ");
        sscanf(pch, "%d", &src[j]);
        printf("\t\t\tSource at Line:   %d : %d\n", i, src[j]);
        pch = strtok(NULL, " ");
        sscanf(pch, "%d", &dst[j]);
        printf("\t\t\tDistance at Line: %d : %d\n", i, dst[j]);
        pch = strtok(NULL, " ");
        sscanf(pch, "%d", &weight[j]);
        printf("\t\t\tWeight at Line:   %d : %d\n", i, weight[j]);
        i++;
        j++;
    }
}

void Create_cost_graph(){
    printf("\t-->Creating Cost Graph\n");
    int N = num_nodes + 1;
    int i,j,x,y,w,c; /*cost[N][N];*/
    int minCost = 99; 
    /*source: client_id */
 
    /*init matrix w/ large cost */
    for(i = 1;i < N; i++)
    for(j = 1;j < N; j++)
    cost[i][j] = IN; 

    /*assign weights to cost graph*/
    for (i = 0 ; i < num_links; i++){
        x = src[i] + 1 ;
        y = dst[i] + 1 ;
        w = weight[i];
        cost[x][y] = cost[y][x] = w;
    }
    i = 0;

    for(i = 0; i < numIPs; i++){
        c = dijkstra(client_id+1, servers_id[i]+1);
        if(c < minCost){
            memset(&dst_return_server_IP[0], 0, sizeof(dst_return_server_IP));
            minCost = c;
            strncpy(dst_return_server_IP, servers_IP[i], sizeof(servers_IP[i]));
        }
    }
    printf("\t\tReturn server IP: %s\n", dst_return_server_IP);
    printf("\t\tMinimum Cost: %d\n", minCost); 
}

int dijkstra(int source, int target){
    printf("\t-->Inside Dijkstra's\n");
    int N = num_nodes + 1;
    int dist[N],prev[N],i,m,min,start,d,j;
    int selected[N]; /*={0};*/
    memset(selected, 0, N*sizeof(int));

    char path[N];
    for(i=1;i< N;i++){
        dist[i] = IN;
        prev[i] = -1;
    }
    start = source;
    selected[start]=1;
    dist[start] = 0;
    while(selected[target] == 0) {
        min = IN;
        m = 0;
        for(i=1;i< N;i++) {
            d = dist[start] + cost[start][i];
            if(d< dist[i]&&selected[i]==0) {
                dist[i] = d;
                prev[i] = start;
            }
            if(min>dist[i] && selected[i]==0){
                min = dist[i];
                m = i;
            }
        }
        start = m;
        selected[start] = 1;
    }
    start = target;
    j = 0;
    while(start != -1){
        path[j++] = start+65;
        start = prev[start];
    }
    path[j]='\0';
    return dist[target];
}