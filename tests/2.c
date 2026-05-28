/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "base/base.h"

#include <stdio.h>

static SocketError err;

const char *testname = "socket_getaddrinfo & socket_getnameinfo test";

void printdnsinfo(const char *nodename, const char *servicename, const SocketDNSRequest *req)
{
    SocketDNSResponse *resps;
    if ((err = socket_getaddrinfo(nodename, servicename, req, &resps)) != SocketError_Success) handlesockerror(err, "socket_getaddrinfo");

    printf("======================================\n   node: %s, service: %s\n======================================\n", nodename, servicename);

    size_t recordn = 0;
    for (SocketDNSResponse *currresp = resps; currresp; currresp = currresp->next)
    {
        printf("Record index #%zu:\n -  Flags: 0x%x (0%Xh).\n -  Address family: ", recordn, currresp->flags, currresp->flags);

        switch (currresp->af)
        {
            case SocketAddressFamily_Unspecified:
                puts("(unspecified).");
                break;

            case SocketAddressFamily_IPv4:
                puts("IPv4.");
                break;

            case SocketAddressFamily_IPv6:
                puts("IPv6.");
                break;

            default:
                puts("(unknown/unsupported).");
        }

        printf(" -  Socket type: ");
        switch (currresp->type)
        {
            case SocketType_Unspecified:
                puts("(any).");
                break;

            case SocketType_Stream:
                puts("stream.");
                break;

            case SocketType_Datagram:
                puts("datagram.");
                break;

            default:
                puts("(unknown/unsupported).");
        }

        printf(" -  Protocol: ");
        switch (currresp->protocol)
        {
            case SocketProtocol_Unspecified:
                puts("(any).");
                break;

            case SocketProtocol_TCP:
                puts("TCP.");
                break;
            
            case SocketProtocol_UDP:
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
            if ((err = socket_unpacksockaddr(currresp->sockaddr, currresp->af, ipaddr, &port)) != SocketError_Success) handlesockerror(err, "socket_unpacksockaddr");

            static char addrstr[1024];
            if ((err = socket_addrtostr(ipaddr, currresp->af, addrstr, sizeof(addrstr))) != SocketError_Success) handlesockerror(err, "socket_addrtostr");

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
        .flags = SOCKET_AI_FLAG_CANONNAME,
        .af = SocketAddressFamily_IPv4,
        .type = SocketType_Stream,
        .protocol = SocketProtocol_TCP
    };

    printdnsinfo("github.com", "http", &req);

    // ===========================================================================================================================================

    SocketIPv4Address saddr;
    IPv4Address addr4 = IPV4ADDR_INIT(IPV4ADDR_PACK(127, 0, 0, 1));
    if ((err = socket_packsockaddr(&saddr, SocketAddressFamily_IPv4, &addr4, 9418)) != SocketError_Success) handlesockerror(err, "socket_packsockaddr");

    char nodename[SOCKET_NI_HOSTMAXSTRSIZE];
    char servicename[SOCKET_NI_SERVMAXSTRSIZE];
    if ((err = socket_getnameinfo(&saddr, sizeof(saddr), nodename, sizeof(nodename), servicename, sizeof(servicename), SOCKET_NI_NOFLAGS)) != SocketError_Success) handlesockerror(err, "socket_getnameinfo");

    printf("127.0.0.1:9418 resolved to (service | node) %s | %s.\n", servicename, nodename);
}
