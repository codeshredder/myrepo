#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <fcntl.h>
#include <net/if.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/udp.h>


#define BUFFER_SIZE  1514

typedef struct psd_header{   //伪头部，用于计算校验和
    unsigned int s_ip;       //source ip
    unsigned int d_ip;       //dest ip
    unsigned char mbz;       //0
    unsigned char proto;     //proto type
    unsigned short plen;     //length
}psd_header;

char test_pkt_arp[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x90, 0xe2, 0xba, 0x83,  0x20, 0xf4, 0x08, 0x06,
                       0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x90, 0xe2,  0xba, 0x83, 0x20, 0xf4,
                       0x0a, 0x8f, 0x25, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x0a, 0x8f, 0x25, 0xd2,
                       0x11, 0x22, 0x33, 0x44};

unsigned short checksum(unsigned short* buffer, int size)//校验和
{
    unsigned long cksum = 0;

    while(size>1)
    {
        cksum += *buffer++;
        size -= sizeof(unsigned short);
    }
    if(size)
    {
        cksum += *(unsigned char*)buffer;
    }

    cksum = (cksum>>16) + (cksum&0xffff); //将高16bit与低16bit相加
    cksum += (cksum>>16); //将进位到高位的16bit与低16bit 再相加

    return (unsigned short)(~cksum);
}

int main(int argc, char **argv)
{
    int    sockfd;
    struct ifreq ifr;
    struct sockaddr_ll addr;
    int addrlen = sizeof(addr);
    int flags;
    unsigned char *buffer = NULL;

    struct ethhdr *eth_hdr = NULL;
    struct iphdr *ip_hdr = NULL;
    struct udphdr *udp_hdr = NULL;
    char *payload = NULL;
    int length = 0;
    int i = 0;

    if (argc < 2)
    {
        return -1;
    }

    if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, argv[1], sizeof(ifr.ifr_name));
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        printf("ioctl error: %s(errno: %d)\n", strerror(errno),errno);
        close(sockfd);
        return -1;
    }
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifindex;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_halen = 6;

    if ((bind(sockfd, (struct sockaddr *)&addr, addrlen)) < 0)
    {
        printf("bind error: %s(errno: %d)\n", strerror(errno), errno);
        close(sockfd);
        return -1;
    }
#if 0
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
#endif

    while (1)
    {

#if 1
        length = 80;
        buffer = malloc(BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);

        eth_hdr = (struct ethhdr *)buffer;
        eth_hdr->h_dest[0] = 0x8C;
        eth_hdr->h_dest[1] = 0x89;
        eth_hdr->h_dest[2] = 0xA5;
        eth_hdr->h_dest[3] = 0x02;
        eth_hdr->h_dest[4] = 0x2a;
        eth_hdr->h_dest[5] = 0x34;
        eth_hdr->h_source[0] = 0x01;
        eth_hdr->h_source[1] = 0x02;
        eth_hdr->h_source[2] = 0x03;
        eth_hdr->h_source[3] = 0x04;
        eth_hdr->h_source[4] = 0x05;
        eth_hdr->h_source[5] = 0x06;
        eth_hdr->h_proto = htons(0x0800);

        ip_hdr = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        ip_hdr->version = 4;                         /** 版本一般的是 4      **/
        ip_hdr->ihl = sizeof(struct iphdr)>>2;  /** IP数据包的头部长度  **/
        ip_hdr->tos = 0;                       /** 服务类型            **/
        ip_hdr->tot_len = htons(length - sizeof(struct ethhdr));         /** IP数据包的长度      **/
        ip_hdr->id = 0;                        /** 让系统去填写吧      **/
        ip_hdr->frag_off = 0;                       /** 和上面一样,省点时间 **/        
        ip_hdr->ttl = MAXTTL;                  /** 最长的时间   255    **/
        ip_hdr->protocol = IPPROTO_UDP;               /** 我们要发的是 TCP包  **/ 
        ip_hdr->check = 0;                       /** 校验和让系统去做    **/
        ip_hdr->saddr = inet_addr("192.168.0.200");     //源地址
        ip_hdr->daddr = inet_addr("192.168.0.106");       //目的地址
        ip_hdr->check = checksum((unsigned short*)ip_hdr, sizeof(struct iphdr));

        udp_hdr = (struct udphdr *)(buffer + sizeof(struct ethhdr) + sizeof(struct iphdr));
        udp_hdr->source = htons(80);
        udp_hdr->dest = htons(1024);
        udp_hdr->len = htons(length - sizeof(struct ethhdr) - sizeof(struct iphdr));
        udp_hdr->check = 0;

        payload = (char *)buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);

        for (i = 0; i < (length - sizeof(struct ethhdr) - sizeof(struct iphdr) - sizeof(struct udphdr)); i++)
        {
            payload[i] = i;
        }

        /* udp checksum */
        psd_header psd;
        psd.s_ip = ip_hdr->saddr;
        psd.d_ip = ip_hdr->daddr;
        psd.mbz = 0;
        psd.proto = 0x11;
        psd.plen = udp_hdr->len;
        char tmp[sizeof(psd) + ntohs(udp_hdr->len)];
        memcpy(tmp, &psd, sizeof(psd));
        memcpy(tmp+sizeof(psd), udp_hdr, ntohs(udp_hdr->len));
        udp_hdr->check = checksum((unsigned short*)tmp, sizeof(tmp));

#else

#endif
        //send
        printf("send: length:%d\r\n", length);
        for (i = 0; i < length; i++)
        {
            if (i%16 == 0)
            {
                printf("\r\n");
            }
            printf("%02x ", buffer[i]);
        }
        printf("\r\n");

        if(sendto(sockfd, buffer, length, 0, (struct sockaddr *)&addr, addrlen) < 0)
        {
            printf("sendto error: %s(errno: %d)\n", strerror(errno), errno);
        }

#if 0
        //recv
        memset(buffer, 0, BUFFER_SIZE);
        length = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
        if (length > 0)
        {
            printf("recv: length:%d\r\n", length);
            for (i = 0; i < length; i++)
            {
                if (i%16 == 0)
                {
                    printf("\r\n");
                }
                printf("%02x ", buffer[i]);
            }
            printf("\r\n");
        }
#endif

        sleep(1);
    }

    close(sockfd);
    return 0;
}

