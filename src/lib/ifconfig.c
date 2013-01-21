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
        printf(host);
    }
    else
        return INET_ADDR_ERROR;


    return inet_addr(host);
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
