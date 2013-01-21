#include "lib/receive.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lib/ifconfig.h"

#define RECEIVE_ERROR -1
#define RECEIVE_ERROR_SETSOCKOPT -2


int
join_to_multicast(int socket, in_addr_t interface, char mcast_ip[])
{
    struct ip_mreq group;
    group.imr_multiaddr.s_addr = inet_addr(mcast_ip);
    group.imr_interface.s_addr = interface;
    if(setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        perror("Adding multicast group error");
        close(socket);
        exit(1);
    }
    else
        printf("Adding multicast group...OK.\n");

    return 0;
}

int
create_socket_for_receive_datagram(struct ifaddrs* ifa,  char mcastIP[], int port)
{
    in_addr_t host = get_inet_addr(ifa);
    if (host == 0)
    {
        printf("Get inet addres error\n");
        return RECEIVE_ERROR;
    }

    // Создаём сокет для UDP
    int sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }
    else
        printf("Opening the datagram socket...OK.\n");

    /* Enable SO_REUSEADDR to allow multiple instances of this */
    /* application to receive copies of the multicast datagrams. */
    {
        int reuse = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
        {
            perror("Setting SO_REUSEADDR error");
            close(sd);
            exit(1);
        }
        else
            printf("Setting SO_REUSEADDR...OK.\n");
    }

    if (join_to_multicast(sd, host, mcastIP) < 0)
    {
        close(sd);
        return RECEIVE_ERROR_SETSOCKOPT;
    }
    printf("\n\n%o\n\n", host);

    /* Bind to the proper port number with the IP address */
    /* specified as INADDR_ANY. */
    struct sockaddr_in localSock;
    memset((char *) &localSock, 0, sizeof(localSock));
    localSock.sin_family = AF_INET;
    localSock.sin_port = htons(port);
    localSock.sin_addr.s_addr = INADDR_ANY;
    if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
    {
        perror("Binding datagram socket error");
        close(sd);
        exit(1);
    }
    else
        printf("Binding datagram socket...OK.\n");

    return sd;
}
