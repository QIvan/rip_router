#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ifaddrs.h>
#include "lib/send.h"
#include "lib/receive.h"
#include "lib/ifconfig.h"
#include "lib/rip.h"




int main (int argc, char *argv[ ])
{
    print_addresses();

    struct ifaddrs *ifaddr, *ifa;
    int family;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
                   can free list later */

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        ifa = ifa->ifa_next;
        ifa = ifa->ifa_next;
        ifa = ifa->ifa_next;
        ifa = ifa->ifa_next;
        if (ifa->ifa_addr == NULL)
            continue;
        family = ifa->ifa_addr->sa_family;

        if (family != AF_INET)
            continue;
        /* Display interface name and family (including symbolic
                       form of the latter for the common families) */
        printf("%s  address family: %d%s\n",
               ifa->ifa_name, family,
               (family == AF_PACKET) ? " (AF_PACKET)" :
                                       (family == AF_INET) ?   " (AF_INET)" :
                                                               (family == AF_INET6) ?  " (AF_INET6)" : "");
        send_packet_in_addr(ifa, RIP_IP, RIP_PORT, "Hello!\0");
        break;
    }
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
