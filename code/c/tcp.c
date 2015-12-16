
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
    int server_fd;
    int client_fd;
    struct sockaddr_in server_addr;
    int server_addrlen = sizeof(server_addr);
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    char buffer[BUFLEN];
    int buffer_len = 0;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }
    
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if (bind(server_fd , (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("\r\n bind error: %s(errno: %d)", strerror(errno), errno);
        close(server_fd);
        return;
    }

    if (listen( server_fd, 10) < 0) { 
        printf ("\r\n listen error") ; 
        close(server_fd);
        return; 
    } 

    while (1)
    {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) { 
            printf ("\r\n accept error") ; 
            continue; 
        } 
        printf("\r\n got connection from %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); 

        while (1)
        {
            memset(buffer, 0, BUFLEN);
            if ((buffer_len = recv(client_fd, buffer, BUFLEN, 0)) < 0) { 
                printf("\r\n recv error" ) ; 
                continue; 
            }

            if (buffer_len == 0)
            {
                printf("\r\n connect close" ) ; 
                break;
            }
            else
            { 
                printf("\r\n data: %s", buffer) ; 
            }
        }

        close(client_fd);
    }

    close(server_fd); 
}

int main(int argc, char**argv)
{

    if (argc < 2) {
        return 0;
    }

    if (0 == strcmp(argv[1],"server"))
    {
        pthread_t task_thread;

        if (pthread_create(&task_thread, NULL, recv_task, NULL) < 0)
        {
            printf("pthread_create failed!\r\n");
            return -1;
        }

        pthread_join(task_thread, NULL);
    }
    else if (0 == strcmp(argv[1],"client"))
    {
        int sockfd;
        struct sockaddr_in addr;
        int addrlen = sizeof(addr);
        char buffer[BUFLEN];
        int buffer_len = 0;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\r\n create socket error: %s(errno: %d)", strerror(errno), errno);
            return -1;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        addr.sin_addr.s_addr = inet_addr(argv[2]);

        printf("\r\n connect ...");
        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            printf("error");
            return -1;
        }
        printf("ok");

        while (1)
        {

            memset(buffer, 0, BUFLEN);
            strncpy(buffer, "aaa", BUFLEN);
            buffer_len = strlen(buffer);

            printf("\r\n sendto %s:%d. data:%s ", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buffer);

            if (send(sockfd, buffer, buffer_len, 0) < 0)
            {
                printf("sendto error: %s(errno: %d)\n", strerror(errno), errno);
                close(sockfd);
                return -1;
            }

            sleep(3);
        }

        close(sockfd);
    }
    
    return 0;
}
