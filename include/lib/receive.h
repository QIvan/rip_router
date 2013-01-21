#ifndef RECEIVE_H
#define RECEIVE_H
#include <arpa/inet.h>  // for in_addr_t
#include <ifaddrs.h>    // for ifaddrs


/**
 * Устанавливает сокету возможность слушать мультикаст из интерфейса
 */
int
join_to_multicast(int socket, in_addr_t interface, char mcast_ip[]);

/**
 * Создаёт сокет для приёма сообщений
 * (уже присоединённый к мультикасту и с связанный с нужным портом)
 * @return сокет или ошибку
 */
int
create_socket_for_receive_datagram(struct ifaddrs* ifa, char mcastIP[], int port);


#endif // RECEIVE_H
