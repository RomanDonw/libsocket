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
    #include <winsock2.h>

    typedef SOCKET Socket;
    #define InvalidSocket INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>

    typedef int Socket;
    #define InvalidSocket -1
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

Socket socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol);
bool socket_close(Socket socket);

#ifdef __cplusplus
    }
#endif

#endif