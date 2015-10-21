
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <fcntl.h>
#include <net/if.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/udp.h>

#define BUFLEN    2048
#define PORT      8888

void *recv_task(void *arg)
{
	int sockfd = *(int *)arg;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	char buffer[BUFLEN];
	int buffer_len = 0;

	while(1) {
		memset(buffer, 0, BUFLEN);
		if ((buffer_len = recvfrom(sockfd, buffer, BUFLEN, 0, (struct sockaddr *) &addr, &addrlen)) < 0)
		{
		    printf("recvfrom error: %s(errno: %d)\n", strerror(errno), errno);
			continue;
		}

		printf("recvfrom %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		printf("data: %s\n" , buffer);
	}
}

int main(int argc, char**argv)
{
	int sockfd;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	char buffer[BUFLEN];
	int buffer_len = 0;

    if (argc < 2) {
		return 0;
    }

    if (0 == strcmp(argv[1],"server")) {

	    pthread_t task_thread;

		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
	        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return -1;
		}
		
		memset((char *) &addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(PORT);              // listen port
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		
		//bind socket to port
		if( bind(sockfd , (struct sockaddr*)&addr, sizeof(addr) ) == -1)
		{
	        printf("bind error: %s(errno: %d)\n", strerror(errno), errno);
      		close(sockfd);
	        return -1;
	    }

	    if (pthread_create(&task_thread, NULL, recv_task, &sockfd) < 0)
	    {
	        printf("pthread_create failed!\r\n");
      		close(sockfd);
	        return -1;
	    }

	    pthread_join(task_thread, NULL);

	    close(sockfd);
	}
	else if (0 == strcmp(argv[1],"client")) {

		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
	        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return -1;
		}

		memset(buffer, 0, BUFLEN);
        strncpy(buffer, "aaa", BUFLEN);
    	buffer_len = strlen(buffer);

    	addr.sin_family = AF_INET;
    	addr.sin_port = htons(PORT);               // dest port
    	addr.sin_addr.s_addr = inet_addr(argv[2]); //dest ip

		if (sendto(sockfd, buffer, buffer_len, 0, (struct sockaddr*) &addr, addrlen) < -1)
		{
		    printf("sendto error: %s(errno: %d)\n", strerror(errno), errno);
			close(sockfd);
			return -1;
		}

		printf("sendto %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		printf("data: %s\n" , buffer);

		close(sockfd);
	}
    
    return 0;
}
