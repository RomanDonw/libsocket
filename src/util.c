#include "util.h"

int fillsockaddrstruct(struct sockaddr *out_sockaddr, SocketAddressFamily af, const char *addr, unsigned short port)
{
    int ret;

    memset(out_sockaddr, 0, sizeof(struct sockaddr));

    switch (af)
    {
        case IPv4:;
            struct sockaddr_in sa;
            sa.sin_family = af;
            sa.sin_port = htons(port);

            ret = inet_pton(af, addr, &sa.sin_addr);

            *out_sockaddr = *((struct sockaddr *)(&sa));
            break;

        case IPv6:;
            struct sockaddr_in6 sa6;
            sa6.sin6_family = af;
            sa6.sin6_port = htons(port);

            ret = inet_pton(af, addr, &sa6.sin6_addr);

            *out_sockaddr = *((struct sockaddr *)(&sa6));
            break;
    }
    return ret;
}