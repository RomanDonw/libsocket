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
    #include <ws2tcpip.h>

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

SocketError socket_getlasterror(void);
const char *socket_strerror(SocketError errcode);

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol);
bool socket_close(Socket *socket);

bool socket_connect(const Socket *socket, const char *address, unsigned short port);

bool socket_bind(const Socket *socket, const char *address, unsigned short port);
bool socket_listen(const Socket *socket, int backlog);
Socket *socket_accept(const Socket *socket);

ssize_t socket_recv(const Socket *socket, void *buffer, size_t len, int flags);
ssize_t socket_send(const Socket *socket, const void *data, size_t len);

bool socket_ioctl(const Socket *socket, SocketIOCTLOption option, void *value);
bool socket_shutdown(const Socket *socket, SocketShutdownMode mode);

SocketAddressFamily socket_getaf(const Socket *socket);
SocketType socket_gettype(const Socket *socket);
SocketProtocol socket_getprotocol(const Socket *socket);

bool socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, socklen_t *optlen);
bool socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, socklen_t optlen);

#ifdef LIBSOCKET_ALLOWUNSAFEACCESS
    SOCKETDESCRIPTOR socket_gethandle(const Socket *socket);
#endif

#ifdef __cplusplus
    }
#endif

#endif