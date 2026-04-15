#include "libsocket.h"

#include <stdlib.h>
#include <string.h>

#ifdef OS_WINDOWS
    #include <ws2tcpip.h>

    void __attribute__((constructor(101))) init()
    {
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        if (WSAStartup(version, &data)) abort();

        if (data.wVersion != version) { WSACleanup(); abort(); }
    }

    void __attribute__((destructor)) cleanup()
    {
        WSACleanup();
    }
#else
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

struct Socket
{
    SOCKETDESCRIPTOR desc;

    SocketAddressFamily af;
    SocketType type;
    SocketProtocol protocol;
};

int fillsockaddrstruct(struct sockaddr *out_sockaddr, SocketAddressFamily af, const char *addr, unsigned short port)
{
    int ret;

    memset(out_sockaddr, 0, sizeof(struct sockaddr));

    switch (af)
    {
        case IPv4:;
            struct sockaddr_in sa;
            sa.sin_family = af;
            sa.sin_port = htons(port);

            ret = inet_pton(af, addr, &sa.sin_addr);

            *out_sockaddr = *((struct sockaddr *)(&sa));
            break;

        case IPv6:;
            struct sockaddr_in6 sa6;
            sa6.sin6_family = af;
            sa6.sin6_port = htons(port);

            ret = inet_pton(af, addr, &sa6.sin6_addr);

            *out_sockaddr = *((struct sockaddr *)(&sa6));
            break;
    }
    return ret;
}

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    SOCKETDESCRIPTOR desc;
    if ((desc = socket(af, type, protocol)) == INVALID_SOCKET) return NULL;

    Socket *ret = malloc(sizeof(Socket));
    if (!ret) return NULL;

    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;
    ret->desc = desc;

    return ret;
}

bool socket_close(Socket *socket)
{
    #ifdef OS_WINDOWS
        if (closesocket(socket->desc)) return false;
    #else
        if (close(socket->desc)) return false;
    #endif

    free(socket);

    return true;
}

bool socket_listen(const Socket *socket, int backlog) { return !listen(socket->desc, backlog); }

bool socket_connect(const Socket *socket, const char *address, unsigned short port)
{
    struct sockaddr sa;
    if (fillsockaddrstruct(&sa, socket->af, address, port) <= 0) return false;
    return !connect(socket->desc, &sa, sizeof(sa));
}

bool socket_bind(const Socket *socket, const char *address, unsigned short port)
{
    struct sockaddr sa;
    if (fillsockaddrstruct(&sa, socket->af, address, port) <= 0) return false;
    return !bind(socket->desc, &sa, sizeof(sa));
}

Socket *socket_accept(const Socket *socket)
{
    SOCKETDESCRIPTOR desc;
    if ((desc = accept(socket->desc, NULL, NULL)) == INVALID_SOCKET) return NULL;

    Socket *ret = malloc(sizeof(Socket));
    if (!ret) return NULL;

    ret->af = socket->af;
    ret->type = socket->type;
    ret->protocol = socket->protocol;
    ret->desc = desc;

    return ret;
}

ssize_t socket_recv(const Socket *socket, void *buffer, size_t len, int flags) { return recv(socket->desc, buffer, len, flags); }
ssize_t socket_send(const Socket *socket, const void *data, size_t len) { return send(socket->desc, data, len, 0); }

bool socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value)
{
    #ifdef OS_WINDOWS
        return !ioctlsocket(socket->desc, option, value);
    #else
        return !ioctl(socket->desc, option, value);
    #endif
}

bool socket_shutdown(const Socket *socket, SocketShutdownMode mode) { return !shutdown(socket->desc, mode); }

SocketAddressFamily socket_getaf(const Socket *socket) { return socket->af; }
SocketType socket_gettype(const Socket *socket) { return socket->type; }
SocketProtocol socket_getprotocol(const Socket *socket) { return socket->protocol; }