#include <stdio.h>
#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

struct interfaces_info
{
    int count;

};

int print_addresses(const int domain)
{
    struct ifconf ifconf;
    struct ifreq ifr[50];
    ifconf.ifc_buf = (char *) ifr;
    ifconf.ifc_len = sizeof ifr;

    int s = socket(domain, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 0;
    }

    if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
        perror("ioctl");
        return 0;
    }

    int if_count = ifconf.ifc_len / sizeof(ifr[0]);
    printf("interfaces = %d:\n", if_count);

    for (int i = 0; i < if_count; i++) {
        char ip[INET_ADDRSTRLEN];
        struct sockaddr_in *s_in = (struct sockaddr_in *) &ifr[i].ifr_addr;

        if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip))) {
            perror("inet_ntop");
            return 0;
        }

        printf("%s - %s\n", ifr[i].ifr_name, ip);
    }

    close(s);

    return 1;
}

int main(int argc, char *argv[])
{
    if (!print_addresses(AF_INET))
        return 1;

    return 0;
}
