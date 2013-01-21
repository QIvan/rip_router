#include "lib/ifconfig.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
// for print_addresses_ioctl
#include <sys/ioctl.h>
#include <net/if.h>


#define INET_ADDR_ERROR 0


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
    }
    else
        return INET_ADDR_ERROR;


    return inet_addr(host);
}

void
print_addresses()
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
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

        /* For an AF_INET* interface address, display the address */

        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                  sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            printf("\taddress: <%s>\n", host);
        }
    }

    freeifaddrs(ifaddr);
}



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

        printf("%s\n", ifr[i].ifr_name);
    }

    close(s);

    return 1;
}
