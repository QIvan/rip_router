#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include "lib/send.h"
#include "lib/receive.h"
#include "lib/ifconfig.h"
#include "lib/rip.h"




int main (int argc, char *argv[ ])
{
    print_addresses();
    printf("\n_______________________________________\n\n\n");

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
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        family = ifa->ifa_addr->sa_family;

        //выбираем какой-нибудь интерфейс кроме локальной петли.
        if(strcmp(ifa->ifa_name, "lo") != 0)
            break;
    }
    send_packet_in_addr(ifa, RIP_IP, RIP_PORT, "Hello!\0");
    int sd = create_socket_for_receive_datagram(ifa, RIP_IP, RIP_PORT);

    char databuf[BUFLEN];
    if(read(sd, databuf, sizeof(databuf)) < 0)
    {
        perror("Reading datagram message error");
        close(sd);
        exit(1);
    }
    else
    {
        printf("Reading datagram message...OK.\n");
        printf("The message from multicast server is: \"%s\"\n", databuf);
    }
    close(sd);

    freeifaddrs(ifaddr);

    return 0;
}
