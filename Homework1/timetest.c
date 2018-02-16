
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

clock_t start, end, duration;

int main(){
	start = clock();
	while(1){
		if((fabs((double)(clock() - start) / CLOCKS_PER_SEC )- 3) >= .00000001){
            break;
    	}
    }
    	/*sleep(3);*/
	end = clock();
	printf("Time Elapsed: %f\n",(double) (end - start) / CLOCKS_PER_SEC);

	struct timeval t_start, t_end, temp;
	gettimeofday(&t_start, NULL);

	while(1){
		gettimeofday(&temp,NULL);
		if(temp.tv_sec - t_start.tv_sec >= 3){
			break;
		}
	}

	gettimeofday(&t_end, NULL);
	printf("Time Elapsed: %ld\n",(t_end.tv_sec - t_start.tv_sec));

	char buffer[100] = "Hellothere my name is Bob and this is a most wond";
	char t1[30],t2[30],t3[20];
	
	sscanf(buffer,"%s %s %s",t1,t2,t3);
	printf("%s\n", t1);
	printf("%s\n", t2);
	printf("%s\n", t3);

	return 0;
}