
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


int main(int argc, char**argv)
{
        int sockfd;
        struct sockaddr_in seraddr;
        struct sockaddr_in cliaddr;
        int serlen = sizeof(seraddr);
        int clilen = sizeof(cliaddr);

        char buffer[BUFLEN];
        int buffer_len;
        int i;

        //create a UDP socket
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
        }

        // zero out the structure
        memset((char *) &seraddr, 0, sizeof(seraddr));

        seraddr.sin_family = AF_INET;
        seraddr.sin_port = htons(PORT);
        seraddr.sin_addr.s_addr = htonl(INADDR_ANY);

        //bind socket to port
        if( bind(sockfd , (struct sockaddr*)&seraddr, sizeof(seraddr) ) == -1)
        {
        printf("bind error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

        //keep listening for data
        while(1)
        {
#if 1
                memset(buffer, 0, BUFLEN);
        strncpy(buffer, "aaa", BUFLEN);
        buffer_len = strlen(buffer);

        cliaddr.sin_family = AF_INET;
        cliaddr.sin_port = htons(PORT);
        cliaddr.sin_addr.s_addr = inet_addr("172.22.13.100");

                if (sendto(sockfd, buffer, buffer_len, 0, (struct sockaddr*) &cliaddr, clilen) < -1)
                {
                    printf("sendto error: %s(errno: %d)\n", strerror(errno), errno);
                        continue;
                }
#endif

#if 1
                printf("Waiting for data...");
                fflush(stdout);

                memset(buffer, 0, BUFLEN);
                if ((buffer_len = recvfrom(sockfd, buffer, BUFLEN, 0, (struct sockaddr *) &cliaddr, &clilen)) < 0)
                {
                    printf("recvfrom error: %s(errno: %d)\n", strerror(errno), errno);
                        continue;
                }

                printf("recvfrom %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                printf("Data: %s\n" , buffer);
#endif
        }

        close(sockfd);
        return 0;
}
