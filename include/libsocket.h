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
    // Windows environment.

    #include <winsock2.h>
    #include <ws2tcpip.h>

    #ifdef LIBSOCKET_STATIC
        #ifdef _MSC_VER
            #define LIBSOCKET_API
        #else
            #define LIBSOCKET_API __attribute__((visibility("default")))
        #endif
    #else
        #ifdef _MSC_VER
            #ifdef LIBSOCKET_EXPORT
                #define LIBSOCKET_API __declspec(dllexport)
            #else
                #define LIBSOCKET_API __declspec(dllimport)
            #endif
        #else
            #define LIBSOCKET_API __attribute__((visibility("default")))
        #endif
    #endif

    typedef SSIZE_T ssize_t;

    typedef SOCKET SOCKETDESCRIPTOR;

    enum
    {
        OnlyRecv = SD_RECEIVE,
        OnlySend = SD_SEND,
        Both = SD_BOTH
    } typedef SocketShutdownMode;

#else
    // POSIX environment.

    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>

    #define LIBSOCKET_API __attribute__((visibility("default")))

    typedef int SOCKETDESCRIPTOR;
    #define INVALID_SOCKET -1

    enum
    {
        OnlyRecv = SHUT_RD,
        OnlySend = SHUT_WR,
        Both = SHUT_RDWR
    } typedef SocketShutdownMode;

#endif

#define LIBSOCKET_ABI __cdecl

enum
{
    IPv4 = AF_INET,
    IPv6 = AF_INET6
} typedef SocketAddressFamily;

enum
{
    Stream = SOCK_STREAM/*,
    Datagram = SOCK_DGRAM*/
} typedef SocketType;

enum
{
    TCP = IPPROTO_TCP/*,
    UDP = IPPROTO_UDP*/
} typedef SocketProtocol;

enum
{
    NonBlockingIO = FIONBIO,
    AvailableDataToRead = FIONREAD
} typedef SocketIOCTLOption;

enum
{
    SocketLevel = SOL_SOCKET,
    TCPLevel = IPPROTO_TCP
} typedef SocketOptionLevel;

enum
{
    // SocketLevel
    Socket_RevcBufferSize = SO_RCVBUF, // int, readable/writable.
    Socket_SendBufferSize = SO_SNDBUF, // int, readable/writable.
    Socket_KeepAliveConnection = SO_KEEPALIVE, // int (bool), readable/writable.
    Socket_AcceptConnections = SO_ACCEPTCONN, // int (bool), readonly.
    Socket_InternalError = SO_ERROR, // int, readonly.
    Socket_AllowReuseAddress = SO_REUSEADDR, // int (bool), readable/writable.
    Socket_Broadcast = SO_BROADCAST, // int (bool), readable/writable.

    // TCPLevel
    TCP_DisableDelay = TCP_NODELAY, // int (bool), readable/writable.
    TCP_MaximumDataSegmentSize = TCP_MAXSEG, // int, readable/writable.
} typedef SocketOptionName;

enum
{
    // Internal (generated only by libsocket).
    InternalUnknownError,
    ParsingAddressFailed,

    // External with internal usage:
    MemoryAllocationFailed, // ENOMEM

    // "Pure-external" (cross-platform).
    Interrupted, // EINTR
    AccessDenied, // EACCES
    Fault, // EFAULT
    IncorrectArgumentValue, // EINVAL
    TooManyOpenedSockets, // EMFILE
    TemporaryUnavailable, // EAGAIN/EWOULDBLOCK
    OperationInProgress, // EINPROGRESS
    InExecutionProcess, // EALREADY
    UnsupportedAddressFamily, // EAFNOSUPPORT
    UnsupportedProtocol, // EPROTONOSUPPORT
    UnsupportedSocketType, // ESOCKTNOSUPPORT
    AddressInUse, // EADDRINUSE
    NetworkUnreachable, // ENETUNREACH
    NetworkDown, // ENETDOWN
    NetworkReset, // ENETRESET
    ConnectionReset, // ECONNRESET
    ConnectionRefused, // ECONNREFUSED
    ConnectionTimedOut, // ETIMEDOUT
    NotConnected, // ENOTCONN
    InvalidDescriptor, // EBADF

    // Windows-specific.
    InitializationError // WSANOTINITIALISED
} typedef SocketError;

#define RECV_NOFLAGS 0
#define RECV_FLAG_PEEK MSG_PEEK
#define RECV_FLAG_WAITALL MSG_WAITALL

typedef struct Socket Socket;

typedef struct sockaddr_in SocketIPv4Address;
typedef struct sockaddr_in6 SocketIPv6Address;
typedef struct sockaddr_storage SocketAddressInterface;
typedef SocketAddressInterface SocketAddress;

typedef struct in_addr IPv4Address;
typedef struct in6_addr IPv6Address;

#define IPV4ADDRESS_ANY { .s_addr = htonl(INADDR_ANY) }
#define IPV4ADDRESS_LOOPBACK { .s_addr = htonl(INADDR_LOOPBACK) }
#define IPV4ADDRESS_BROADCAST { .s_addr = htonl(INADDR_BROADCAST) }

#define IPV6ADDRESS_ANY IN6ADDR_ANY_INIT
#define IPV6ADDRESS_LOOPBACK IN6ADDR_LOOPBACK_INIT

union
{
    IPv4Address ipv4;
    IPv6Address ipv6;
} typedef IPAddressInterface;

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getlasterror(void);
LIBSOCKET_API const char * LIBSOCKET_ABI socket_strerror(SocketError errcode);

LIBSOCKET_API bool LIBSOCKET_ABI socket_parseaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr);
LIBSOCKET_API bool LIBSOCKET_ABI socket_fillsockaddr(SocketAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port);

LIBSOCKET_API Socket * LIBSOCKET_ABI socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol);
LIBSOCKET_API bool LIBSOCKET_ABI socket_close(Socket *socket);

LIBSOCKET_API bool LIBSOCKET_ABI socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr);
LIBSOCKET_API bool LIBSOCKET_ABI socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr);

LIBSOCKET_API bool LIBSOCKET_ABI socket_listen(const Socket *socket, int backlog);
LIBSOCKET_API Socket * LIBSOCKET_ABI socket_accept(const Socket *socket);

LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_recv(const Socket *socket, void *buffer, size_t len, int flags);
LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_send(const Socket *socket, const void *data, size_t len);

LIBSOCKET_API bool LIBSOCKET_ABI socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value);
LIBSOCKET_API bool LIBSOCKET_ABI socket_shutdown(const Socket *socket, SocketShutdownMode mode);

LIBSOCKET_API SocketAddressFamily LIBSOCKET_ABI socket_getaf(const Socket *socket);
LIBSOCKET_API SocketType LIBSOCKET_ABI socket_gettype(const Socket *socket);
LIBSOCKET_API SocketProtocol LIBSOCKET_ABI socket_getprotocol(const Socket *socket);

LIBSOCKET_API bool LIBSOCKET_ABI socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen);
LIBSOCKET_API bool LIBSOCKET_ABI socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen);

#if defined(LIBSOCKET_ALLOWUNSAFEACCESS) || defined(LIBSOCKET_EXPORT)
    LIBSOCKET_API SOCKETDESCRIPTOR LIBSOCKET_ABI socket_gethandle(const Socket *socket);
#endif

#ifdef __cplusplus
    }
#endif

#endif
