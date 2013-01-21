#ifndef IFCONFIG_H
#define IFCONFIG_H

#include <arpa/inet.h>  // для in_addr_t
#include <ifaddrs.h>    // для ifaddrs

/**
 * Извлекает из ifaddrs ip-адрес в формате in_addr_t (uint_32)
 * (для получения строкой в классическом виде смотри inet_ntop )
*/
in_addr_t
get_inet_addr(struct ifaddrs* ifa);





/**
 * @deprecated
 * эта версия получает адрес через ioctl
 * хз как лучше, но навреное через getifaddrs,
 * ибо она работает на freebsd
 */
int print_addresses_ioctl(const int domain);

#endif // IFCONFIG_H
