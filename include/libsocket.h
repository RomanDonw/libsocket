#ifndef LIBSOCKET_H
#define LIBSOCKET_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#include <stdbool.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    #define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
    // windows env.

    #include <winsock2.h>
    typedef SSIZE_T ssize_t;

    typedef SOCKET SOCKETDESCRIPTOR;

    enum
    {
        OnlyRecv = SD_RECEIVE,
        OnlySend = SD_SEND,
        Both = SD_BOTH
    } typedef SocketShutdownMode;

#else
    // unix env.

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <sys/ioctl.h>

    typedef int SOCKETDESCRIPTOR;
    #define INVALID_SOCKET -1

    enum
    {
        OnlyRecv = SHUT_RD,
        OnlySend = SHUT_WR,
        Both = SHUT_RDWR
    } typedef SocketShutdownMode;

#endif

enum
{
    IPv4 = AF_INET,
    IPv6 = AF_INET6
} typedef SocketAddressFamily;

enum
{
    Stream = SOCK_STREAM,
    Datagram = SOCK_DGRAM
} typedef SocketType;

enum
{
    TCP = IPPROTO_TCP,
    UDP = IPPROTO_UDP
} typedef SocketProtocol;

enum
{
    NonBlockingIO = FIONBIO,
    AvailableDataToRead = FIONREAD
} typedef SocketIOCTLOption;

typedef struct Socket Socket;

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol);
bool socket_close(Socket *socket);

bool socket_connect(const Socket *socket, const char *address, unsigned short port);

bool socket_bind(const Socket *socket, const char *address, unsigned short port);
bool socket_listen(const Socket *socket, int backlog);
Socket *socket_accept(const Socket *socket);

// add flags arg. to this functions:
ssize_t socket_recv(const Socket *socket, void *buffer, size_t len);
ssize_t socket_send(const Socket *socket, const void *data, size_t len);

bool socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value);
bool socket_shutdown(const Socket *socket, SocketShutdownMode mode);

SocketAddressFamily socket_getaf(const Socket *socket);
SocketType socket_gettype(const Socket *socket);
SocketProtocol socket_getprotocol(const Socket *socket);

#ifdef __cplusplus
    }
#endif

#endif