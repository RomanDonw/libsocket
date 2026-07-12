/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#define LIBNSOCKET_ALLOWUNSAFEACCESS

#include "base.h"

#include <stdio.h>
#include <stdlib.h>

static NError err;

void printipv4(IPv4Address addr, const char *addrname)
{
    static char addrstr[IPV4ADDRSTRSIZE];
    if ((err = nsocket_ipaddrtostr(&addr, NSocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != NError_Success) handlesockerror(err, "nsocket_addrtostr");
    printf("%s: %s\n", addrname, addrstr);
}

const char *testname = "libnnsocket API generic test";

void test(void)
{
    NSocket *s;
    if ((err = nsocket_open(&s, NSocketAddressFamily_IPv4, NSocketType_Stream, NSocketProtocol_TCP)) != NError_Success) handlesockerror(err, "nsocket_open");
    puts(" === === [nsocket opened] === ===\n");

    printf("NSocket descriptor: %lli\n", (long long)nsocket_gethandle(s));

    puts("");

    {
        NSocketIPv4Address saddr;
        IPv4Address addr = IPV4ADDR_ANY;
        unsigned short port = 12345;
        char addrstr[IPV4ADDRSTRSIZE];

        // display address to bind, pack NSocketAddress structure & bind to them.
        if ((err = nsocket_ipaddrtostr(&addr, NSocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != NError_Success) handlesockerror(err, "nsocket_addrtostr");
        printf("Binding to address %s:%u...\n", addrstr, port);
        if ((err = nsocket_packsockipaddr(&saddr, NSocketAddressFamily_IPv4, &addr, port)) != NError_Success) handlesockerror(err, "nsocket_packsockaddr");
        if ((err = nsocket_bind(s, &saddr, sizeof(saddr))) != NError_Success) handlesockerror(err, "nsocket_bind");

        // fill address & port variables with garbage.
        addr = IPV4ADDR_BROADCAST;
        port = 0;

        // back unpack NSocketAddress struct & display unpacked address and port.
        if ((err = nsocket_unpacksockipaddr(&saddr, NSocketAddressFamily_IPv4, &addr, &port)) != NError_Success) handlesockerror(err, "nsocket_unpacksockaddr");
        if ((err = nsocket_ipaddrtostr(&addr, NSocketAddressFamily_IPv4, addrstr, IPV4ADDRSTRSIZE)) != NError_Success) handlesockerror(err, "nsocket_addrtostr");
        printf("Binded to address %s:%u.\n", addrstr, port);

        switch (NSOCKET_GETSOCKADDRAF(&saddr))
        {
            case NSocketAddressFamily_IPv4:
                puts("NSocket has IPv4 address family.");
                break;

            case NSocketAddressFamily_IPv6:
                puts("NSocket has IPv6 address family.");
                break;

            default:
                puts("!!! UNKNOWN SOCKET ADDRESS FAMILY !!!");
        }

        puts("");
    }

    NSocketLingerOptions ling =
    {
        .enable = true,
        .linger = 5
    };
    if ((err = nsocket_setopt(s, NSocketOptionLevel_Socket, NSocketOptionName_Socket_Linger, &ling, sizeof(ling))) != NError_Success) handlesockerror(err, "nsocket_setopt");

    ling.enable = false;
    ling.linger = 666;

    size_t lingsz = sizeof(NSocketLingerOptions);
    if ((err = nsocket_getopt(s, NSocketOptionLevel_Socket, NSocketOptionName_Socket_Linger, &ling, &lingsz)) != NError_Success) handlesockerror(err, "nsocket_getopt");
    printf("sizeof(NSocketLingerOptions) = %zu    |    lingsz from nsocket_getopt = %zu\n", sizeof(ling), lingsz);
    if (lingsz != sizeof(ling)) testabort_c("lingsz != sizeof(ling). TEST NOT PASSED. lingsz must be equal to sizeof(ling)! Testing aborted.");

    printf("ling.enable = %s\nling.linger = %u seconds.\n", ling.enable ? "true" : "false", ling.linger);

    if ((err = nsocket_close(s)) != NError_Success) handlesockerror(err, "nsocket_close");
    puts(" === === [nsocket closed] === ===\n");

    printf("htons: before(0x%x) -> after(0x%x)\n", 0x1234, NSOCKET_HTONS(0x1234));
    printf("htonl: before(0x%x) -> after(0x%x)\n", 0x12345678, NSOCKET_HTONL(0x12345678));

    puts("");

    {
        IPv4Address addr = IPV4ADDR_INIT(IPV4ADDR_PACK(123, 213, 45, 67));

        printipv4(IPV4ADDR_ANY, "IPv4 any");
        printipv4(IPV4ADDR_LOOPBACK, "IPv4 loopback");
        printipv4(IPV4ADDR_BROADCAST, "IPv4 broadcast");
        printipv4(addr, "Custom (must be 123.213.45.67)");
    }
}
