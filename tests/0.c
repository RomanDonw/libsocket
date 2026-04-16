#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "libsocket.h"

#ifdef OS_WINDOWS
    #include <windows.h>
    #define MILLIS(x) Sleep(x);
#else
    #include <unistd.h>
    #define MILLIS(x) usleep(x * 1000)
#endif

int getsocksendbuffsize(const Socket *s)
{
    int sendbuffsize;
    
    socklen_t sendbuffsize_len = sizeof(sendbuffsize);
    if (!socket_getopt(s, SocketLevel, SendBufferSize, &sendbuffsize, &sendbuffsize_len)) { puts("socket_getopt error"); abort(); }
    if (sendbuffsize_len != sizeof(sendbuffsize)) { puts("sendbuffsize_len != sizeof(sendbuffsize). Abort."); abort(); }

    return sendbuffsize;
}

void setsocksendbuffsize(const Socket *s, int size)
{
    if (!socket_setopt(s, SocketLevel, SendBufferSize, &size, sizeof(size))) { puts("socket_setopt error. Abort."); abort(); }
}

int main(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) { puts("socket_open error"); abort(); }

    printf("Old send buffer size: %i\n", getsocksendbuffsize(s));
    setsocksendbuffsize(s, 4096);
    printf("New send buffer size: %i\n", getsocksendbuffsize(s));

    if (!socket_connect(s, "127.0.0.1", 8000)) { puts("socket_connect error"); abort(); }

    const char *request = "GET / HTTP/1.0\r\n\r\n";
    if (!socket_send(s, request, strlen(request))) { puts("socket_send abort"); abort(); }

    MILLIS(100);

    unsigned long avail;
    if (!socket_ioctl(s, AvailableDataToRead, &avail)) { puts("socket_ioctl error"); abort(); }
    printf("Available bytes: %lu\n", avail);

    const size_t BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    ssize_t readbytes;
    while ((readbytes = socket_recv(s, buffer, BUFFER_SIZE, RECV_NOFLAGS)) > 0)
    {
        for (size_t i = 0; i < readbytes; i++) putchar(buffer[i]);
    }

    if (!socket_close(s)) { puts("socket_close error"); abort(); }

    return 0;
}