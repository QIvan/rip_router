#include "lib/send.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "lib/ifconfig.h"

#define NO_SEND -1
#define NO_SEND_HOST -2
#define NO_SEND_SETSOCKOPT -3
#define SET_MULTICAST_ERROR -1
#define INET_ADDR_ERROR 0



int
disable_loopback(int socket)
{
// Пока отключим для отладки
//    char loopch = 0;
//    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loopch, sizeof(loopch)) < 0)
//    {
//        perror("Setting IP_MULTICAST_LOOP error");
//        return IP_MULTICAST_LOOP_ERROR;
//    }
//    else
//        printf("Disabling the loopback...OK.\n");
    return 0;
}

int
set_if_for_multicast(int socket, in_addr_t interface)
{
    // Адрес должен быть обязательно наш (текущей машины)
    /// @todo лучше делать так, а не через ip_mreqn imr_ifindex
    struct in_addr localInterface;
    localInterface.s_addr = interface;
    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        perror("Setting local interface error");
        return SET_MULTICAST_ERROR;
    }
    else
        printf("Setting the local interface...OK\n");

    return 0;
}

int
create_socket_for_send(struct ifaddrs* ifa)
{
    in_addr_t host = get_inet_addr(ifa);
    if (host == 0)
    {
        printf("Get inet addres error\n");
        return NO_SEND_HOST;
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


    if ((disable_loopback(sd) < 0) || (set_if_for_multicast(sd, host) < 0))
    {
        close(sd);
        return NO_SEND_SETSOCKOPT;
    }

    return sd;
}

int
send_packet_in_addr(struct ifaddrs* ifa, char mcastIP[], int port, char data[])
{
    printf("\nSend to intarface: %s\n", ifa->ifa_name);
    int sd = create_socket_for_send(ifa);
    if(sd < 0)
    {
        return NO_SEND;
    }

    // Инициализуруем группу для мультикаста
    struct sockaddr_in groupSock;
    memset((char *) &groupSock, 0, sizeof(groupSock));
    groupSock.sin_family = AF_INET;
    groupSock.sin_addr.s_addr = inet_addr(mcastIP);
    groupSock.sin_port = htons(port);

    // Отправка сообщения
    /// @todo хз почему, но размер нужно + 2 (наверное для символа окончания строки)
    if(sendto(sd, data, sizeof(data)+2, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0)
    {
        perror("Sending datagram message error");
        return NO_SEND;
    }
    else
        printf("Sending datagram message...OK\n");

    close(sd);
    return 0;
}
