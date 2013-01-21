#include "lib/rip.h"
#include "stdio.h"
#include <arpa/inet.h>

int
mask_len(struct in_addr mask) {
    int len = 0;
    int bit_pos = 0;
    while ((bit_pos < 32) && (mask.s_addr & (1 << bit_pos))) {
        len++; bit_pos++;
    }
    return len;
}


void
dump_rip(char *buffer, int len) {
    struct rip_route *rt = (struct rip_route *)(buffer + 4);
    while ((char*) rt < buffer + len) {
        printf("\tAFI %d addr %s/%d, tag 0x%04x, metric %u",
               rt->addr_family, inet_ntoa(rt->addr), mask_len(rt->mask),
               rt->tag, ntohl(rt->metric));
        printf(",  nexthop: %s\n", inet_ntoa(rt->nexthop));
        rt++;
    }
}
