#include "base/base.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int getsocksendbuffsize(const Socket *s)
{
    int sendbuffsize;
    
    socklen_t sendbuffsize_len = sizeof(sendbuffsize);
    if (!socket_getopt(s, SocketLevel, Socket_SendBufferSize, &sendbuffsize, &sendbuffsize_len)) { puts("socket_getopt error"); abort(); }
    if (sendbuffsize_len != sizeof(sendbuffsize)) { puts("sendbuffsize_len != sizeof(sendbuffsize). Abort."); abort(); }

    return sendbuffsize;
}

void setsocksendbuffsize(const Socket *s, int size)
{
    if (!socket_setopt(s, SocketLevel, Socket_SendBufferSize, &size, sizeof(size))) { puts("socket_setopt error. Abort."); abort(); }
}

const char *testname = "HTTP 1.0 GET request to localhost:8000";

void test(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) handlesockerror("socket_open");

    printf("Old send buffer size: %i\n", getsocksendbuffsize(s));
    setsocksendbuffsize(s, 4096);
    printf("New send buffer size: %i\n", getsocksendbuffsize(s));

    IPv4Address localhost = IPV4ADDR_LOOPBACK;
    SocketIPv4Address saddr;
    if (!socket_packsockaddr(&saddr, IPv4, &localhost, 8000)) handlesockerror("socket_packsockaddr");
    if (!socket_connect(s, &saddr, sizeof(saddr))) handlesockerror("socket_connect");

    const char *request = "GET / HTTP/1.0\r\n\r\n";
    if (!socket_send(s, request, strlen(request), SOCKET_SEND_NOFLAGS)) handlesockerror("socket_send");

    waitms(100);

    uint32_t avail;
    if (!socket_ioctl(s, AvailableDataToRead, &avail)) handlesockerror("socket_ioctl");
    printf("Available bytes: %lu\n", avail);

    //const size_t BUFFER_SIZE = 512;
    #define BUFFER_SIZE 512
    char buffer[BUFFER_SIZE];
    ssize_t readbytes;
    while ((readbytes = socket_recv(s, buffer, BUFFER_SIZE, SOCKET_RECV_NOFLAGS)) > 0)
    {
        for (size_t i = 0; i < readbytes; i++) putchar(buffer[i]);
    }

    if (!socket_close(s)) handlesockerror("socket_close");
}
