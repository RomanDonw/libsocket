#include "base/base.h"

#include <stdio.h>

const char *testname = "socket_getaddrinfo & socket_getnameinfo test";

void printdnsinfo(const char *nodename, const char *servicename, const SocketDNSRequest *req)
{
    SocketDNSResponse *resps;
    if (!socket_getaddrinfo(nodename, servicename, req, &resps)) handlesockerror("socket_getaddrinfo");

    printf("======================================\n   node: %s, service: %s\n======================================\n", nodename, servicename);

    size_t recordn = 0;
    for (SocketDNSResponse *currresp = resps; currresp; currresp = currresp->next)
    {
        printf("Record name #%llu:\n -  Flags: 0x%x (0%Xh).\n -  Address family: ", recordn, currresp->flags, currresp->flags);

        switch (currresp->af)
        {
            case UnspecifiedAddressFamily:
                puts("(unspecified).");
                break;

            case IPv4:
                puts("IPv4.");
                break;

            case IPv6:
                puts("IPv6.");
                break;

            default:
                puts("(unknown/unsupported).");
        }

        printf(" -  Socket type: ");
        switch (currresp->type)
        {
            case AnySocketType:
                puts("(any).");
                break;

            case Stream:
                puts("stream.");
                break;

            case Datagram:
                puts("datagram.");
                break;

            default:
                puts("(unknown/unsupported).");
        }

        printf(" -  Protocol: ");
        switch (currresp->protocol)
        {
            case AnyProtocol:
                puts("(any).");
                break;

            case TCP:
                puts("TCP.");
                break;
            
            case UDP:
                puts("UDP.");
                break;

            default:
                puts("(unknown/unsupported).");
        }

        if (currresp->canonname) printf(" -  Canonical name: \"%s\".\n", currresp->canonname);
        else puts(" -  Canonical name: (unspecified).");

        if (currresp->sockaddr && currresp->sockaddrlen)
        {
            static char ipaddr[256];
            static unsigned short port;
            if (!socket_unpacksockaddr(currresp->sockaddr, currresp->af, ipaddr, &port)) handlesockerror("socket_unpacksockaddr");

            static char addrstr[1024];
            if (!socket_addrtostr(ipaddr, currresp->af, addrstr, sizeof(addrstr))) handlesockerror("socket_addrtostr");

            printf(" -  Address: [%s]:%u\n", addrstr, port);
        }
        else puts(" -  Address: (unspecified).");

        if (currresp->next) puts("");
        recordn++;
    }

    socket_freeaddrinfo(resps);
    puts("======================================\n======================================\n======================================\n");
}

void test(void)
{
    printdnsinfo("google.com", NULL, NULL);
    printdnsinfo("wikipedia.org", "80", NULL);
    printdnsinfo("kernel.org", "http", NULL);

    SocketDNSRequest req =
    {
        .flags = AI_CANONNAME,
        .af = IPv4,
        .type = Stream,
        .protocol = TCP
    };

    printdnsinfo("github.com", "http", &req);

    SocketIPv4Address saddr;
    IPv4Address addr4 = IPV4ADDR_INIT(IPV4ADDR_PACK(127, 0, 0, 1));
    if (!socket_packsockaddr(&saddr, IPv4, &addr4, 9418)) handlesockerror("socket_packsockaddr");

    char nodename[NI_MAXHOST];
    char servicename[NI_MAXSERV];
    if (!socket_getnameinfo(&saddr, sizeof(saddr), nodename, sizeof(nodename), servicename, sizeof(servicename), 0)) handlesockerror("socket_getnameinfo");

    printf("127.0.0.1:9418 resolved to (service | node) %s | %s.\n", servicename, nodename);
}