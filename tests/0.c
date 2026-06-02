/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "base/base.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static SocketError err;

int getsocksendbuffsize(const Socket *s)
{
    int sendbuffsize;
    socklen_t sendbuffsize_len = sizeof(sendbuffsize);
    if ((err = socket_getopt(s, SocketOptionLevel_Socket, SocketOptionName_Socket_SendBufferSize, &sendbuffsize, &sendbuffsize_len)) != SocketError_Success) handlesockerror(err, "socket_getopt");
    if (sendbuffsize_len != sizeof(sendbuffsize)) testabort_c("sendbuffsize_len != sizeof(sendbuffsize). Abort.");
    return sendbuffsize;
}

void setsocksendbuffsize(const Socket *s, int size)
{
    err = socket_setopt(s, SocketOptionLevel_Socket, SocketOptionName_Socket_RecvBufferSize, &size, sizeof(size));
    if (err != SocketError_Success) handlesockerror(err, "socket_setopt");
}

const char *testname = "HTTP 1.0 GET request to localhost:8000";

void test(void)
{
    Socket *s;
    if ((err = socket_open(&s, SocketAddressFamily_IPv4, SocketType_Stream, SocketProtocol_TCP)) != SocketError_Success) handlesockerror(err, "socket_open");

    printf("Old send buffer size: %i\n", getsocksendbuffsize(s));
    setsocksendbuffsize(s, 4096);
    printf("New send buffer size: %i\n", getsocksendbuffsize(s));

    IPv4Address localhost = IPV4ADDR_LOOPBACK;
    SocketIPv4Address saddr;
    if ((err = socket_packsockaddr(&saddr, SocketAddressFamily_IPv4, &localhost, 8000)) != SocketError_Success) handlesockerror(err, "socket_packsockaddr");
    if ((err = socket_connect(s, &saddr, sizeof(saddr))) != SocketError_Success) handlesockerror(err, "socket_connect");

    const char *request = "GET / HTTP/1.0\r\n\r\n";
    if ((err = socket_send(s, request, strlen(request), NULL, SOCKET_SEND_NOFLAGS)) != SocketError_Success) handlesockerror(err, "socket_send");

    waitms(100);

    size_t avail;
    if ((err = socket_getreadablebytes(s, &avail)) != SocketError_Success) handlesockerror(err, "socket_getreadablebytes");
    printf("Available bytes: %zu\n\n", avail);
    
    #define BUFFER_SIZE 512
    char buffer[BUFFER_SIZE];
    ssize_t readbytes;
    while (true)
    {
        if ((err = socket_recv(s, buffer, BUFFER_SIZE, &readbytes, SOCKET_RECV_NOFLAGS)) != SocketError_Success || readbytes <= 0) break;
        for (size_t i = 0; i < readbytes; i++) putchar(buffer[i]);
    }

    if ((err = socket_close(s)) != SocketError_Success) handlesockerror(err, "socket_close");
}
