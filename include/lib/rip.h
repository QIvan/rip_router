#ifndef RIP_H
#define RIP_H
#include <stdint.h>
#include <netinet/in.h>

#define RIP_IP "224.0.0.9"
#define RIP_PORT 4321   // ВНЕЗАПНО: слушать 520 порт может только root
#define BUFLEN 512


struct rip_route {
    uint8_t code;
    uint8_t addr_family;
    uint16_t tag;
    struct in_addr addr;
    struct in_addr mask;
    struct in_addr nexthop;
    uint32_t metric;
};

/**
 * вычисление размера CIDR-маски
*/
int
mask_len(struct in_addr mask);

/**
 * вывод в консоль содержания RIP-сообщения
 * (сообщение занимает первые `len` байт в буфере)
 */
void
dump_rip(char *buffer, int len);


#endif // RIP_H
