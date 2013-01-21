#ifndef SEND_H
#define SEND_H
#include <arpa/inet.h>  // for in_addr_t
#include <ifaddrs.h>    // for ifaddrs

/**
 * Отключает loopback чтобы не получать собственные дейтаграммы
 * @param socket
*/
int
disable_loopback(int socket);

/**
 * Установка локального интерфейса для исходящих многоадресных дейтаграмм.
 * @param socket
 * @param host - адрес интерфейса для мультикаста @see get_inet_addr
 * @return
 */
int
set_if_for_multicast(int socket, in_addr_t interface);

/**
 * Создаёт сокет для отправки сообщений
 * (уже установленный для широковещательной рассылки
 * и с отключённым loopback)
 */
int
create_socket_for_send(struct ifaddrs* ifa);

/**
 * Отправляет широкрвещательное mes сообщение в интерфейс ifa
 * @param ifa - какой интерфейс использовать при отправке
 * @param mcastIP - ip мультикаста
 * @param port - порт для мульиткаста
 * @param data - данные для передачи
 * @return 0 - успех, NO_SEND - неудача
 * Если не получится создать сокет - убивает программу.
 */
int
send_packet_in_addr(struct ifaddrs* ifa, char mcastIP[], int port, char data[]);

#endif // SEND_H
