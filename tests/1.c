/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#define LIBSOCKET_ALLOWUNSAFEACCESS

#include "base.h"

#include <stdio.h>
#include <stdlib.h>

static SocketError err;

void printipv4(IPv4Address addr, const char *addrname)
{
    static char addrstr[IPV4ADDRSTRSIZE];
    if ((err = socket_ipaddrtostr(&addr, SocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != SocketError_Success) handlesockerror(err, "socket_addrtostr");
    printf("%s: %s\n", addrname, addrstr);
}

const char *testname = "libsocket API generic test";

void test(void)
{
    Socket *s;
    if ((err = socket_open(&s, SocketAddressFamily_IPv4, SocketType_Stream, SocketProtocol_TCP)) != SocketError_Success) handlesockerror(err, "socket_open");
    puts(" === === [socket opened] === ===\n");

    printf("Socket descriptor: %i\n", socket_gethandle(s));

    puts("");

    {
        SocketIPv4Address saddr;
        IPv4Address addr = IPV4ADDR_ANY;
        unsigned short port = 12345;
        char addrstr[IPV4ADDRSTRSIZE];

        // display address to bind, pack SocketAddress structure & bind to them.
        if ((err = socket_ipaddrtostr(&addr, SocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != SocketError_Success) handlesockerror(err, "socket_addrtostr");
        printf("Binding to address %s:%u...\n", addrstr, port);
        if ((err = socket_packsockipaddr(&saddr, SocketAddressFamily_IPv4, &addr, port)) != SocketError_Success) handlesockerror(err, "socket_packsockaddr");
        if ((err = socket_bind(s, &saddr, sizeof(saddr))) != SocketError_Success) handlesockerror(err, "socket_bind");

        // fill address & port variables with garbage.
        addr = IPV4ADDR_BROADCAST;
        port = 0;

        // back unpack SocketAddress struct & display unpacked address and port.
        if ((err = socket_unpacksockipaddr(&saddr, SocketAddressFamily_IPv4, &addr, &port)) != SocketError_Success) handlesockerror(err, "socket_unpacksockaddr");
        if ((err = socket_ipaddrtostr(&addr, SocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != SocketError_Success) handlesockerror(err, "socket_addrtostr");
        printf("Binded to address %s:%u.\n", addrstr, port);

        switch (SOCKET_GETSOCKADDRAF(&saddr))
        {
            case SocketAddressFamily_IPv4:
                puts("Socket has IPv4 address family.");
                break;

            case SocketAddressFamily_IPv6:
                puts("Socket has IPv6 address family.");
                break;

            default:
                puts("!!! UNKNOWN SOCKET ADDRESS FAMILY !!!");
        }

        puts("");
    }

    SocketLingerOptions ling =
    {
        .enable = true,
        .linger = 5
    };
    if ((err = socket_setopt(s, SocketOptionLevel_Socket, SocketOptionName_Socket_Linger, &ling, sizeof(ling))) != SocketError_Success) handlesockerror(err, "socket_setopt");

    ling.enable = false;
    ling.linger = 666;

    size_t lingsz = sizeof(SocketLingerOptions);
    if ((err = socket_getopt(s, SocketOptionLevel_Socket, SocketOptionName_Socket_Linger, &ling, &lingsz)) != SocketError_Success) handlesockerror(err, "socket_getopt");
    printf("sizeof(SocketLingerOptions) = %zu    |    lingsz from socket_getopt = %zu\n", sizeof(ling), lingsz);
    if (lingsz != sizeof(ling)) testabort_c("lingsz != sizeof(ling). TEST NOT PASSED. lingsz must be equal to sizeof(ling)! Testing aborted.");

    printf("ling.enable = %s\nling.linger = %u seconds.\n", ling.enable ? "true" : "false", ling.linger);

    if ((err = socket_close(s)) != SocketError_Success) handlesockerror(err, "socket_close");
    puts(" === === [socket closed] === ===\n");

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
}
