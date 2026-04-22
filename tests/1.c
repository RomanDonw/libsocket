#include <stdio.h>

#define LIBSOCKET_ALLOWUNSAFEACCESS
#include "libsocket.h"
#include "util.h"

void printipv4(IPv4Address addr, const char *addrname)
{
    static char addrstr[IPV4ADDRSTRSIZE];
    if (!socket_addrtostr(&addr, IPv4, addrstr, IPV4ADDRSTRSIZE)) handleerror("socket_addrtostr");
    printf("%s: %s\n", addrname, addrstr);
}

int main(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) handleerror("socket_open");

    printf("Socket descriptor: %i\n", socket_gethandle(s));

    puts("");

    {
        SocketAddress saddr;
        IPv4Address addr = IPV4ADDR_ANY;
        unsigned short port = 12345;
        char addrstr[IPV4ADDRSTRSIZE];

        // display address to bind, pack SocketAddress structure & bind to them.
        if (!socket_addrtostr(&addr, IPv4, addrstr, IPV4ADDRSTRSIZE)) handleerror("socket_addrtostr");
        printf("Binding to address %s:%u...\n", addrstr, port);
        if (!socket_packsockaddr(&saddr, IPv4, &addr, port)) handleerror("socket_packsockaddr");
        if (!socket_bind(s, &saddr)) handleerror("socket_bind");

        // fill address & port variables with garbage.
        addr = IPV4ADDR_BROADCAST;
        port = 0;

        // back unpack SocketAddress struct & display unpacked address and port.
        if (!socket_unpacksockaddr(&saddr, IPv4, &addr, &port)) handleerror("socket_unpacksockaddr");
        if (!socket_addrtostr(&addr, IPv4, addrstr, IPV4ADDRSTRSIZE)) handleerror("socket_addrtostr");
        printf("Binded to address %s:%u.\n", addrstr, port);

        switch (socket_getsockaddraf(&saddr))
        {
            case IPv4:
                puts("Socket has IPv4 address family.");
                break;

            case IPv6:
                puts("Socket has IPv6 address family.");
                break;

            default:
                puts("!!! UNKNOWN SOCKET ADDRESS FAMILY !!!");
        }

        puts("");
    }

    if (!socket_close(s)) handleerror("socket_close");

    printf("htons: before(0x%x) -> after(0x%x)\n", 0x1234, SOCKET_HTONS(0x1234));
    printf("htonl: before(0x%x) -> after(0x%x)\n", 0x12345678, SOCKET_HTONL(0x12345678));

    puts("");

    {
        IPv4Address addr = IPV4ADDR_INIT(IPV4ADDR_PACK(123, 213, 45, 67));

        printipv4(IPV4ADDR_ANY, "IPv4 any");
        printipv4(IPV4ADDR_LOOPBACK, "IPv4 loopback");
        printipv4(IPV4ADDR_BROADCAST, "IPv4 broadcast");
        printipv4(addr, "Custom (must be 123.213.45.67)");
    }

    return 0;
}