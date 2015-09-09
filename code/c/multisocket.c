
#define PRINTF  if (1)   printf


#define BUFFER_SIZE      2048
#define DEVICENAME_LEN   20

typedef enum{
    OVS_PORT_ETH1,
    OVS_PORT_ETH2,
    OVS_PORT_MAX,
}OVS_PORT_ENUM;

#define TAP_VALID  0x5a5a

struct tapInfo
{
    int valid;
    int port;
    int sockfd;
    struct sockaddr_ll addr;
    char devname[DEVICENAME_LEN];
};

struct tapInfo tapList[OVS_PORT_MAX];

unsigned char *ovs_recvbuffer = NULL;

int OVS_Init()
{
    pthread_t task_thread;
    unsigned int arg = 0;

    memset(&tapList, 0, OVS_PORT_MAX* sizeof(struct tapInfo));

    ovs_recvbuffer = malloc(BUFFER_SIZE);
    memset(ovs_recvbuffer, 0, BUFFER_SIZE);

    OVS_AddTap(OVS_PORT_ETH1, "tap-eth1");
    OVS_AddTap(OVS_PORT_ETH2, "tap-eth2");

    if (pthread_create(&task_thread, NULL, OVS_RecvTask, &arg) < 0)
    {
        printf("pthread_create failed!\r\n");
        return -1;
    }

//    pthread_join(task_thread, NULL);

    return 0;
}


int OVS_AddTap(int port, char *devname)
{
    struct tapInfo *tapNode = NULL;
    int    sockfd;
    struct ifreq ifr;
    struct sockaddr_ll addr;
    int flags;

    tapNode = &tapList[port];
    memset(tapNode, 0, sizeof(struct tapInfo));
    tapNode->port = port;
    strncpy(tapNode->devname, devname, DEVICENAME_LEN);

    /* create socket */
    if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, devname, sizeof(ifr.ifr_name));
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        printf("ioctl error: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifindex;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_halen = 6;

    if ((bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    {
        printf("bind error: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }

    if ((flags = fcntl(sockfd, F_GETFL, 0)) < 0)
    {
        printf("fcntl error: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
    if ((fcntl(sockfd, F_SETFL, (flags | O_NONBLOCK))) < 0)
    {
        printf("fcntl error: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }

    tapNode->sockfd = sockfd;
    tapNode->addr = addr;
    tapNode->valid = TAP_VALID;

    return 0;
}

int OVS_DelTap(int port)
{
    struct tapInfo *tapNode = NULL;

    tapNode = &tapList[port];
    memset(tapNode, 0, sizeof(struct tapInfo));

    return 0;
}


void *OVS_RecvTask(void *arg)
{
    fd_set fds;
    struct timeval tv;
    int maxsock = 0;
    int retval = 0;

    struct tapInfo *tapNode = NULL;
    int addrlen = 0;
    int length = 0;
    int i = 0;

    while(1)
    {
        FD_ZERO(&fds);

        for (i = 0; i < OVS_PORT_MAX; i++)
        {
            tapNode = &tapList[i];

            if (TAP_VALID == tapNode->valid)
            {
                FD_SET(tapNode->sockfd, &fds);

                if (maxsock < tapNode->sockfd) {
                    maxsock = tapNode->sockfd;
                }
            }
        }

        // timeout setting
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(maxsock + 1, &fds, NULL, NULL, &tv);
        if (retval < 0) {
            printf("select error\n");
        }
        else if (retval == 0) {
            printf("timeout\n");
        }
        else if (retval) {
            // printf("Data is available now\n");

            for (i = 0; i < OVS_PORT_MAX; i++)
            {
                tapNode = &tapList[i];

                if (TAP_VALID == tapNode->valid)
                {
                     if (FD_ISSET(tapNode->sockfd, &fds)) {
                        PRINTF("OVS_RecvTask: %s\n", tapNode->devname, 0, 0, 0);

                        memset(ovs_recvbuffer, 0, BUFFER_SIZE);
                        addrlen = sizeof(tapNode->addr);

                        length = recvfrom(tapNode->sockfd, ovs_recvbuffer, BUFFER_SIZE, 0, (struct sockaddr *)&tapNode->addr, &addrlen);
                        if (length > 0)
                        {
                            //(void)OVS_Proc(tapNode->port, ovs_recvbuffer, length);
                        }
                    }
                }
            }
        }
    }

}

void OVS_SendPkt(int port, char *buffer, int length)
{
    struct tapInfo *tapNode = &tapList[port];
    int addrlen;
    int i;

#if 1
    SWEET_PRINTF("OVS_SendPkt: port:%d, length:%d\n", port, length);

    for (i = 0; i < length; i++)
    {
        if (i%16 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("%02x ", *(unsigned char *)(buffer + i));
    }
    PRINTF("\r\n");
#endif

    if (TAP_VALID == tapNode->valid)
    {
        addrlen = sizeof(tapNode->addr);
        if(sendto(tapNode->sockfd, buffer, length, 0,(struct sockaddr *)&tapNode->addr, addrlen) < 0)
        {
            PRINTF("sendto error: %s(errno: %d)\n", strerror(errno), errno);
            return;
        }
        PRINTF("sendto: %s, length:%d\r\n", tapNode->devname, length);

        return;
    }
}

