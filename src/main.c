#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <linux/netdevice.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <unistd.h>


#define NO_SEND -1

#define RIP_IP "224.0.0.9"
#define RIP_PORT 503

#define BUFLEN 512

int print_addresses(const int domain)
{
    struct ifconf ifconf;
    struct ifreq ifr[50];
    ifconf.ifc_buf = (char *) ifr;
    ifconf.ifc_len = sizeof ifr;

    int s = socket(domain, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("socket");
        return 0;
    }

    /**
     * Функция ioctl манипулирует базовыми параметрами устройств, представленных в виде специальных файлов.
     * В частности, многими оперативными характеристиками специальных символьных файлов (например терминалов)
     * можно управлять через ioctl запросы.
     * Первый аргумент d - открытый файловый дескриптор.
     * Второй аргумент является кодом запроса, который зависит от устройства.
     * Третий аргумент является указателем на память
    */
    if (ioctl(s, SIOCGIFCONF, &ifconf) == -1)
    {
        perror("ioctl");
        return 0;
    }

    int if_count = ifconf.ifc_len / sizeof(ifr[0]);
    printf("interfaces = %d:\n", if_count);

    for (int i = 0; i < if_count; i++)
    {
        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

        if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip)))
        {
            perror("inet_ntop");
            return 0;
        }

        printf("%s - %i\n", ifr[i].ifr_name, s_in->sin_addr);
    }

    close(s);

    return 1;
}

int send_packet(char * mes)
{
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket() failed");
        return -1;
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(RIP_PORT);

    u_int yes = 1;
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &yes, sizeof(yes));
    if (inet_aton(RIP_IP, &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        return -1;
    }

    sprintf(buf, mes);
    if (sendto(s, buf, BUFLEN, 0, &si_other, slen) == -1)
    {
        perror("sendto() failed");
        return -1;
    }

    close(s);
    return 0;
}

int
send_packet_in_addr(struct ifaddrs* ifa, char databuf[])
{
    int sd;
    //char databuf[1024] = "Multicast test message lol!";
    int datalen = sizeof(databuf);

    char host[NI_MAXHOST];
    if (ifa->ifa_addr->sa_family == AF_INET) {
        int s = getnameinfo(ifa->ifa_addr,
                            sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        if (s != 0) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }
        else
            printf("Received host addres...OK\n");
    }
    else
        return NO_SEND;
    printf("\n\n%s\n\n", host);

    /* Create a datagram socket on which to send. */
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening the datagram socket...OK.\n");


    //Отключаем loopback чтобы не получать собственные датаграммы
    {
        char loopch = 0;
        if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
        {
            perror("Setting IP_MULTICAST_LOOP error");
            close(sd);
            exit(1);
        }
        else
            printf("Disabling the loopback...OK.\n");
    }



    // Подключаемся к multcast рассылке
    // Адрес должен быть обязательно наш (текущей машины)
    /// @todo нужно делать так, а не через ip_mreqn imr_ifindex
    struct in_addr localInterface;
    localInterface.s_addr = inet_addr(host);
    if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
        exit(1);
    }
    else
        printf("Setting the local interface...OK\n");

    /* Initialize the group sockaddr structure with a */
    /* group address of 225.1.1.1 and port 5555. */
    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(RIP_IP);
    groupSock.sin_port = htons(RIP_PORT);

    /* Send a message to the multicast group specified by the*/
    /* groupSock sockaddr structure. */
    if(sendto(sd, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
    {
        perror("Sending datagram message error");
        exit(EXIT_FAILURE);
    }
    else
        printf("Sending datagram message...OK\n");

}


/* размер буфера для входящих датаграмм */
#define BUFSIZE 512

struct rip_route {
    uint8_t code;
    uint8_t addr_family;
    uint16_t tag;
    struct in_addr addr;
    struct in_addr mask;
    struct in_addr nexthop;
    uint32_t metric;
};

/** вычисление размера CIDR-маски */
int
mask_len(struct in_addr mask) {
    int len = 0;
    int bit_pos = 0;
    while ((bit_pos < 32) && (mask.s_addr & (1 << bit_pos))) {
        len++; bit_pos++;
    }
    return len;
}

/** вывод в консоль содержания RIP-сообщения
 *  (сообщение занимает первые `len` байт в буфере)
 */
void
dump_rip(char *buffer, int len) {
    struct rip_route *rt = (struct rip_route *)(buffer + 4);
    while ((char*) rt < buffer + len) {
        printf("\tAFI %d addr %s/%d, tag 0x%04x, metric %u",
               rt->addr_family, inet_ntoa(rt->addr), mask_len(rt->mask),
               rt->tag, ntohl(rt->metric));
        printf(",  nexthop: %s\n", inet_ntoa(rt->nexthop));
        rt++;
    }
}




int main (int argc, char *argv[ ])
{
//    print_addresses(AF_INET);

    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
                   can free list later */

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        /* Display interface name and family (including symbolic
                       form of the latter for the common families) */
        printf("%s  address family: %d%s\n",
               ifa->ifa_name, family,
               (family == AF_PACKET) ? " (AF_PACKET)" :
                                       (family == AF_INET) ?   " (AF_INET)" :
                                                               (family == AF_INET6) ?  " (AF_INET6)" : "");

        send_packet_in_addr(ifa, "Hello!");
    }

    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);

    return 0;
}
