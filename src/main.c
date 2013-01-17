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
#define IP_MULTICAST_LOOP_ERROR -1
#define JOIN_MULTICAST_ERROR -1
#define INET_ADDR_ERROR 0

#define RIP_IP "224.0.0.9"
#define RIP_PORT 503

#define BUFLEN 512

/**
 * эта версия получает адрес через ioctl
 * хз как лучше, но навреное через getifaddrs,
 * ибо она работает на freebsd
 */
int print_addresses_ioctl(const int domain)
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

    /*
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

/**
 * Извлекает из ifaddrs ip-адрес в формате in_addr_t (uint_32)
 * (для получения строкой в классическом виде смотри inet_ntop )
*/
in_addr_t
get_inet_addr(struct ifaddrs* ifa)
{
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
        printf(host);
    }
    else
        return INET_ADDR_ERROR;


    return inet_addr(host);
}

/**
 * Отключает loopback чтобы не получать собственные датаграммы
 * @param socket
*/
int
disable_loopback(int socket)
{
    char loopch = 0;
    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
    {
        perror("Setting IP_MULTICAST_LOOP error");
        return IP_MULTICAST_LOOP_ERROR;
    }
    else
        printf("Disabling the loopback...OK.\n");
}

/**
 * Подключается к multicast-рассылке
 * @param socket
 * @param host - адрес интерфейса для мультикаста @see get_inet_addr
 * @return
 */
int
join_to_multicast(int socket, in_addr_t host)
{
    // Адрес должен быть обязательно наш (текущей машины)
    /// @todo лучше делать так, а не через ip_mreqn imr_ifindex
    struct in_addr localInterface;
    localInterface.s_addr = host;
    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
        return JOIN_MULTICAST_ERROR;
    }
    else
        printf("Setting the local interface...OK\n");
}

/**
 * Отправляет широкрвещательное mes сообщение в интерфейс ifa
 * @return 0 - успех, NO_SEND - неудача
 * Если не получится создать сокет - убивает программу.
 */
int
send_packet_in_addr(struct ifaddrs* ifa, char mes[])
{
    in_addr_t host = get_inet_addr(ifa);
    if (host == 0)
    {
        printf("Get inet addres error\n");

        return NO_SEND;
    }
    printf("\n\n%i\n\n", host);

    // Создаём сокет для UDP
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening the datagram socket...OK.\n");


    if ((disable_loopback(sd) < 0) || (join_to_multicast(sd, host) < 0))
    {
        close(sd);
        return NO_SEND;
    }



    // Инициализуруем группу для мультикаста
    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(RIP_IP);
    groupSock.sin_port = htons(RIP_PORT);

    // Отправка сообщения
    if(sendto(sd, mes, sizeof(mes), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
    {
        perror("Sending datagram message error");
        exit(EXIT_FAILURE);
    }
    else
        printf("Sending datagram message...OK\n");

    return 0;
}


int
receive_datagram(struct ifaddrs* ifa)
{


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
//        receive_datagram(ifa);
    }


    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);

    return 0;
}
