#ifndef LIBSOCKET_H
#define LIBSOCKET_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#include <stdbool.h>
#include <stdint.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    #define LIBSOCKET_OS_WINDOWS
#endif

#ifdef LIBSOCKET_OS_WINDOWS
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

    #define LIBSOCKET_ABI __cdecl

    #define LIBSOCKET_WINSOCK_VERSION_HIGH 2
    #define LIBSOCKET_WINSOCK_VERSION_LOW 2

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
    #include <netinet/in.h>
    #include <netinet/tcp.h>

    #define LIBSOCKET_API __attribute__((visibility("default")))
    #define LIBSOCKET_ABI

    typedef int SOCKETDESCRIPTOR;
    #define INVALID_SOCKET -1

    enum
    {
        OnlyRecv = SHUT_RD,
        OnlySend = SHUT_WR,
        Both = SHUT_RDWR
    } typedef SocketShutdownMode;

#endif

#define SOCKET_HTONS(x) (((uint16_t)(x) & 0xFF00) >> 8) | (((uint16_t)(x) & 0x00FF) << 8)
#define SOCKET_HTONL(x) ((((uint32_t)(x) & 0xFF000000) >> 24) | (((uint32_t)(x) & 0x000000FF) << 24) | (((uint32_t)(x) & 0x00FF0000) >> 8) | (((uint32_t)(x) & 0x0000FF00) << 8))

#define SOCKET_NTOHS(x) SOCKET_HTONS(x)
#define SOCKET_NTOHL(x) SOCKET_HTONL(x)

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
    NonBlockingIO, // bool, writeonly.
    AvailableDataToRead // uint32_t, readonly.
} typedef SocketIOCTLOption;

enum
{
    SocketLevel = SOL_SOCKET,
    TCPLevel = IPPROTO_TCP
} typedef SocketOptionLevel;

enum
{
    // SocketLevel
    Socket_RecvBufferSize = SO_RCVBUF, // int, readable/writable.
    Socket_SendBufferSize = SO_SNDBUF, // int, readable/writable.
    Socket_KeepAliveConnection = SO_KEEPALIVE, // int (bool), readable/writable.
    Socket_AcceptConnections = SO_ACCEPTCONN, // int (bool), readonly.
    Socket_InternalError = SO_ERROR, // int, readonly.
    Socket_AllowReuseAddress = SO_REUSEADDR, // int (bool), readable/writable.
    Socket_Broadcast = SO_BROADCAST, // int (bool), readable/writable.
    Socket_Linger = SO_LINGER, // struct SocketLingerOptions, readable/writable.
    Socket_RecvTimeout = SO_RCVTIMEO, // uint32_t (milliseconds), readable/writable.
    Socket_SendTimeout = SO_SNDTIMEO, // uint32_t (milliseconds), readable/writable.

    // TCPLevel
    TCP_DisableDelay = TCP_NODELAY, // int (bool), readable/writable.
    TCP_MaximumDataSegmentSize = TCP_MAXSEG, // int, readable/writable.
} typedef SocketOptionName;

enum
{
    InternalUnknownError,
    ParsingAddressFailed,

    MemoryAllocationFailed, // ENOMEM
    Interrupted, // EINTR
    AccessDenied, // EACCES
    Fault, // EFAULT
    IncorrectArgumentValue, // EINVAL
    TooManyOpenedSockets, // EMFILE
    WouldBlock, // EAGAIN/EWOULDBLOCK
    OperationInProgress, // EINPROGRESS
    InExecutionProcess, // EALREADY
    UnsupportedAddressFamily, // EAFNOSUPPORT
    UnsupportedProtocol, // EPROTONOSUPPORT
    UnsupportedSocketType, // ESOCKTNOSUPPORT
    AddressInUse, // EADDRINUSE
    AddressNotAvailable, // EADDRNOTAVAIL
    NetworkUnreachable, // ENETUNREACH
    NetworkDown, // ENETDOWN
    NetworkReset, // ENETRESET
    ConnectionReset, // ECONNRESET
    ConnectionRefused, // ECONNREFUSED
    ConnectionAborted, // ECONNABORTED
    ConnectionTimedOut, // ETIMEDOUT
    NotConnected, // ENOTCONN
    AlreadyConnected, // EISCONN
    InvalidDescriptor, // EBADF
    NoSpaceLeft, // ENOSPC
    ProtocolOptionUnsupported, // ENOPROTOOPT
    OperationNotSupported, // EOPNOTSUPP
    SystemBufferOverflowed, // ENOBUFS
    CannotTranslateName, // ELOOP
    DestinationAddressRequired, // EDESTADDRREQ
    NameTooLong, // ENAMETOOLONG

    // Windows-specific.
    InitializationError // WSANOTINITIALISED
} typedef SocketError;

struct
{
    bool enable;
    unsigned short linger; // in seconds.
} typedef SocketLingerOptions;

#define SOCKET_RECV_NOFLAGS 0
#define SOCKET_RECV_FLAG_PEEK MSG_PEEK
#define SOCKET_RECV_FLAG_WAITALL MSG_WAITALL
#define SOCKET_RECV_FLAG_TRUNC MSG_TRUNC

#define SOCKET_SEND_NOFLAGS 0
#define SOCKET_SEND_FLAG_DONTROUTE MSG_DONTROUTE

typedef struct Socket Socket;

typedef struct sockaddr_in SocketIPv4Address;
typedef struct sockaddr_in6 SocketIPv6Address;
typedef struct sockaddr_storage SocketAddressStorage;
typedef void SocketAddressInterface;

typedef struct in_addr IPv4Address;
typedef struct in6_addr IPv6Address;
typedef void IPAddressInterface;

#define IPV4ADDRSTRSIZE INET_ADDRSTRLEN
#define IPV6ADDRSTRSIZE INET6_ADDRSTRLEN

#define IPV4ADDR_INIT(addr) { .s_addr = SOCKET_HTONL(addr) }
#define IPV4ADDR_PACK(a, b, c, d) ((((uint32_t)(a) & 0xFF) << 24) | (((uint32_t)(b) & 0xFF) << 16) | (((uint32_t)(c) & 0xFF) << 8) | ((uint32_t)(d) & 0xFF))

LIBSOCKET_API extern const IPv4Address IPV4ADDR_ANY;
LIBSOCKET_API extern const IPv4Address IPV4ADDR_LOOPBACK;
LIBSOCKET_API extern const IPv4Address IPV4ADDR_BROADCAST;

LIBSOCKET_API extern const IPv6Address IPV6ADDR_ANY;
LIBSOCKET_API extern const IPv6Address IPV6ADDR_LOOPBACK;

LIBSOCKET_API extern void *(*libsocket_malloc)(size_t);
LIBSOCKET_API extern void *(*libsocket_realloc)(void *, size_t);
LIBSOCKET_API extern void (*libsocket_free)(void *);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getlasterror(void);
LIBSOCKET_API const char * LIBSOCKET_ABI socket_strerror(SocketError errcode);

LIBSOCKET_API bool LIBSOCKET_ABI socket_parseaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr);
LIBSOCKET_API bool LIBSOCKET_ABI socket_addrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size);

LIBSOCKET_API SocketAddressFamily LIBSOCKET_ABI socket_getsockaddraf(const SocketAddressInterface *sockaddr);
LIBSOCKET_API bool LIBSOCKET_ABI socket_packsockaddr(SocketAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port);
LIBSOCKET_API bool LIBSOCKET_ABI socket_unpacksockaddr(const SocketAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port);

LIBSOCKET_API Socket * LIBSOCKET_ABI socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol);
LIBSOCKET_API bool LIBSOCKET_ABI socket_close(Socket *socket);

LIBSOCKET_API bool LIBSOCKET_ABI socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);
LIBSOCKET_API bool LIBSOCKET_ABI socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);

LIBSOCKET_API bool LIBSOCKET_ABI socket_listen(const Socket *socket, int backlog);
// [socket_accept]: sockaddr & sockaddrlen can be NULL. see <sys/socket.h> accept function documentation for more info.
LIBSOCKET_API Socket * LIBSOCKET_ABI socket_accept(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen);

LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_recv(const Socket *socket, void *buffer, size_t len, int flags);
// [socket_recvfrom]: sockaddr & sockaddrlen can be NULL. see <sys/socket.h> recvfrom function documentation for more info.
LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_recvfrom(const Socket *socket, void *buffer, size_t len, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen);
LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_send(const Socket *socket, const void *data, size_t len, int flags);
LIBSOCKET_API ssize_t LIBSOCKET_ABI socket_sendto(const Socket *socket, const void *buffer, size_t len, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);

LIBSOCKET_API bool LIBSOCKET_ABI socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value);
LIBSOCKET_API bool LIBSOCKET_ABI socket_shutdown(const Socket *socket, SocketShutdownMode mode);

LIBSOCKET_API bool LIBSOCKET_ABI socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen);
LIBSOCKET_API bool LIBSOCKET_ABI socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen);

LIBSOCKET_API SocketAddressFamily LIBSOCKET_ABI socket_getaf(const Socket *socket);
LIBSOCKET_API SocketType LIBSOCKET_ABI socket_gettype(const Socket *socket);
LIBSOCKET_API SocketProtocol LIBSOCKET_ABI socket_getprotocol(const Socket *socket);

LIBSOCKET_API bool LIBSOCKET_ABI socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size);
LIBSOCKET_API bool LIBSOCKET_ABI socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size);

#if defined(LIBSOCKET_ALLOWUNSAFEACCESS) || defined(LIBSOCKET_EXPORT)
    LIBSOCKET_API SOCKETDESCRIPTOR LIBSOCKET_ABI socket_gethandle(const Socket *socket);
#endif

#ifdef __cplusplus
    }
#endif

#endif
