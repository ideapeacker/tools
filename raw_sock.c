#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
 * 计算校验和的函数
 * @ptr 待计算校验和的数据
 * @nbytes 数据长度
 * @return 返回校验和
 */
unsigned short check_sum(unsigned short *ptr, int nbytes)
{
    register long sum;
    unsigned short oddbyte;
    register short answer;

    sum = 0;
    while (nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1)
    {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return (answer);
}

int main(int argc, char *argv[])
{
    int sock, val;
    char buf[1024];
    // IP包头
    struct iphdr *iph = (struct ip *)buf;
    // ICMP包头
    struct icmphdr *icmph = (struct icmphdr *)(iph + 1);

    socklen_t addr_len;
    struct sockaddr_in dst;
    struct sockaddr_in src_addr, dst_addr;

    if (argc < 3)
    {
        printf("\nUsage: %s <saddress> <dstaddress>\n", argv[0]);
        return 0;
    }

    bzero(buf, sizeof(buf));

    // 创建原始套接字
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        perror("socket() error");
        /* If something wrong, just exit */
        return -1;
    }

    val = 1;
    // 告诉内核我们自己填充IP头部
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &val, sizeof(val)) < 0)
    {
        perror("setsockopt() for IP_HDRINCL error");
        return -1;
    }

    // 填充IP头部
    iph->ihl = 5;     // ip头部的长度/4
    iph->version = 4; // 版本信息
    iph->tos = 0;

    //memset(buf + 28, 'A', 12);
    iph->tot_len = sizeof(struct iphdr) +
                   sizeof(struct icmphdr) + 10; // 总长度等于ip头部+icmp总长度
    iph->id = htons(4321);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->check = 0; // 让内核自己去计算校验和
    iph->saddr = inet_addr(argv[1]);
    iph->daddr = inet_addr(argv[2]);
    // check sum
    // iph->check = check_sum((unsigned short *)buf, iph->tot_len);

    dst.sin_addr.s_addr = iph->daddr;
    dst.sin_family = AF_INET;

    // 添加ICMP包头
    icmph->type = ICMP_ECHO;
    icmph->code = 0;
    icmph->checksum = 0;
    icmph->un.echo.id = htons(9987);
    icmph->un.echo.sequence = htons(9988);

    // 首部检验和
    icmph->checksum = check_sum((void *)icmph, sizeof(struct icmphdr));

    addr_len = sizeof(dst);

    // 发数据
    val = sendto(sock, buf, iph->tot_len, 0, (struct sockaddr *)&dst, addr_len);
    if (val < 0)
    {
        perror("sendto() error\n");
    }
    else
    {
        printf("sendto() is OK\n");
    }

    // 收数据
    val = recvfrom(sock, buf + 30, 5, 0, (struct sockaddr *)&dst, &addr_len);
    if (val < 0)
    {
        perror("recv from error");
    }
    else
    {
        printf("recv %d bytes data\n", val);
        iph = (void *)(buf + 30);
        icmph = (struct icmphdr *)(iph + 1);
        printf("icmp type: %d, icmp code = %d, seq = %u, id = %u\n",
               icmph->type, icmph->code, ntohs(icmph->un.echo.sequence),
               ntohs(icmph->un.echo.id));
    }

    // 关闭socket
    close(sock);

    return 0;
};