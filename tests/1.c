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

    IPv4Address addr;
    char addrstr[IPV4ADDRSTRSIZE];

    printipv4(IPV4ADDR_ANY, "IPv4 any");
    printipv4(IPV4ADDR_LOOPBACK, "IPv4 loopback");
    printipv4(IPV4ADDR_BROADCAST, "IPv4 broadcast");

    return 0;
}