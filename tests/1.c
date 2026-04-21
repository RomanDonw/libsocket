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

    if (!socket_close(s)) handleerror("socket_close");

    printf("htons: before(0x%x) -> after(0x%x)\n", 0x1234, SOCKET_HTONS(0x1234));
    printf("htonl: before(0x%x) -> after(0x%x)\n", 0x12345678, SOCKET_HTONL(0x12345678));

    IPv4Address addr = IPV4ADDR_INIT(IPV4ADDR_PACK(4, 3, 2, 1));

    printipv4(IPV4ADDR_ANY, "IPv4 any");
    printipv4(IPV4ADDR_LOOPBACK, "IPv4 loopback");
    printipv4(IPV4ADDR_BROADCAST, "IPv4 broadcast");
    printipv4(addr, "Custom (must be 123.456.789.952)");

    return 0;
}