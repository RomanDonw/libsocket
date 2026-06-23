/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef LIBSOCKET_H
#define LIBSOCKET_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    #define LIBSOCKET_OS_WINDOWS
#endif

#if defined(__BYTE_ORDER__)
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define LIBSOCKET_BIG_ENDIAN
    #elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define LIBSOCKET_LITTLE_ENDIAN
    #endif

#elif defined(__BIG_ENDIAN__) || defined(__BIG_ENDIAN) || defined(_BIG_ENDIAN)
    #define LIBSOCKET_BIG_ENDIAN

#elif defined(LIBSOCKET_OS_WINDOWS)
    #if defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM) || defined(_M_ARM64)
        #define LIBSOCKET_LITTLE_ENDIAN
    #endif
    
#endif

#if !defined(LIBSOCKET_LITTLE_ENDIAN) && !defined(LIBSOCKET_BIG_ENDIAN)
    #error Unsupported/unknown CPU architecture endianness.
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

    typedef SOCKET SOCKETDESCRIPTOR;

#else
    // POSIX environment.

    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <netdb.h>
    
    #define LIBSOCKET_API __attribute__((visibility("default")))

    typedef int SOCKETDESCRIPTOR;
    #define INVALID_SOCKET -1

#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define LIBSOCKET_ABI

#define LIBSOCKET_WINSOCK_DEFAULT_VERSION (((2) << 8) | (2))

#if defined(LIBSOCKET_LITTLE_ENDIAN)
    #define SOCKET_HTONS(x) ((uint16_t)(((uint16_t)(x) & 0xFF00) >> 8) | (((uint16_t)(x) & 0x00FF) << 8))
    #define SOCKET_HTONL(x) ((uint32_t)((((uint32_t)(x) & 0xFF000000) >> 24) | (((uint32_t)(x) & 0x000000FF) << 24) | (((uint32_t)(x) & 0x00FF0000) >> 8) | (((uint32_t)(x) & 0x0000FF00) << 8)))
#elif defined(LIBSOCKET_BIG_ENDIAN)
    #define SOCKET_HTONS(x) ((uint16_t)(x))
    #define SOCKET_HTONL(x) ((uint32_t)(x))
#endif

#define SOCKET_NTOHS(x) SOCKET_HTONS(x)
#define SOCKET_NTOHL(x) SOCKET_HTONL(x)

enum SocketAddressFamily
{
    SocketAddressFamily_Unspecified = AF_UNSPEC,
    SocketAddressFamily_IPv4 = AF_INET,
    SocketAddressFamily_IPv6 = AF_INET6
} typedef SocketAddressFamily;

enum SocketType
{
    SocketType_Unspecified = 0,
    SocketType_Stream = SOCK_STREAM,
    SocketType_Datagram = SOCK_DGRAM,
} typedef SocketType;

enum SocketProtocol
{
    SocketProtocol_Unspecified = 0,
    SocketProtocol_TCP = IPPROTO_TCP,
    SocketProtocol_UDP = IPPROTO_UDP
} typedef SocketProtocol;

enum SocketOptionLevel
{
    SocketOptionLevel_Socket = SOL_SOCKET,
    SocketOptionLevel_IP = IPPROTO_IP,
    SocketOptionLevel_TCP = IPPROTO_TCP,
} typedef SocketOptionLevel;

/*
        R/O - readonly.
        W/O - writeonly.
        R/W - readable/writeable.
*/

enum SocketOptionName
{
    // Socket level.
    SocketOptionName_Socket_RecvBufferSize = SO_RCVBUF, // int, R/W.
    SocketOptionName_Socket_SendBufferSize = SO_SNDBUF, // int, R/W.
    SocketOptionName_Socket_KeepAliveConnection = SO_KEEPALIVE, // bool, R/W.
    SocketOptionName_Socket_AcceptConnections = SO_ACCEPTCONN, // bool, R/O.
    SocketOptionName_Socket_InternalError = SO_ERROR, // int, R/O.
    SocketOptionName_Socket_Broadcast = SO_BROADCAST, // bool, R/W.
    SocketOptionName_Socket_Linger = SO_LINGER, // struct SocketLingerOptions, R/W.
    SocketOptionName_Socket_RecvTimeout = SO_RCVTIMEO, // uint32_t (milliseconds), R/W.
    SocketOptionName_Socket_SendTimeout = SO_SNDTIMEO, // uint32_t (milliseconds), R/W.
    SocketOptionName_Socket_Type = SO_TYPE, // int (enum SocketType), R/O.

    // IP level.
    SocketOptionName_IP_TimeToLive = IP_TTL, // uint8_t, R/W.

    // TCP level.
    SocketOptionName_TCP_NoDelay = TCP_NODELAY, // bool, R/W.
    SocketOptionName_TCP_MaxKeepAliveProbes = TCP_KEEPCNT, // int, R/W.
    SocketOptionName_TCP_KeepAliveProbesInterval = TCP_KEEPINTVL, // int (seconds), R/W.
    SocketOptionName_TCP_ConnectionKeepIdleTime = TCP_KEEPIDLE, // int (seconds), R/W.
    SocketOptionName_TCP_FastOpen = TCP_FASTOPEN, // bool, R/W.
} typedef SocketOptionName;

enum SocketError
{
    SocketError_Success = 0,

    SocketError_NotInitialized,
    SocketError_AlreadyInitialized,
    SocketError_InternalUnknownError,
    SocketError_InternalSizeMismatch,
    SocketError_InternalVariableOverflow,
    SocketError_ParsingAddressFailed,
    SocketError_MutexAPIError,

    SocketError_MemoryAllocationFailed, // ENOMEM/EAI_MEMORY
    SocketError_Interrupted, // EINTR
    SocketError_AccessDenied, // EACCES
    SocketError_Fault, // EFAULT
    SocketError_InsufficientBufferSize, // ERANGE
    SocketError_IncorrectArgumentValue, // EINVAL
    SocketError_TooManyOpenedSockets, // EMFILE
    SocketError_WouldBlock, // EAGAIN/EWOULDBLOCK
    SocketError_OperationInProgress, // EINPROGRESS
    SocketError_InExecutionProcess, // EALREADY
    SocketError_UnsupportedAddressFamily, // EAFNOSUPPORT
    SocketError_UnsupportedProtocol, // EPROTONOSUPPORT
    SocketError_UnsupportedSocketType, // ESOCKTNOSUPPORT
    SocketError_UnsupportedProtocolOption, // ENOPROTOOPT
    SocketError_UnsupportedOperation, // EOPNOTSUPP
    SocketError_AddressInUse, // EADDRINUSE
    SocketError_AddressNotAvailable, // EADDRNOTAVAIL
    SocketError_NetworkUnreachable, // ENETUNREACH
    SocketError_NetworkDown, // ENETDOWN
    SocketError_NetworkReset, // ENETRESET
    SocketError_ConnectionReset, // ECONNRESET
    SocketError_ConnectionRefused, // ECONNREFUSED
    SocketError_ConnectionAborted, // ECONNABORTED
    SocketError_ConnectionTimedOut, // ETIMEDOUT
    SocketError_NotConnected, // ENOTCONN
    SocketError_AlreadyConnected, // EISCONN
    SocketError_InvalidDescriptor, // EBADF
    SocketError_NoSpaceLeft, // ENOSPC
    SocketError_SystemBufferOverflowed, // ENOBUFS
    SocketError_CannotTranslateName, // ELOOP
    SocketError_DestinationAddressRequired, // EDESTADDRREQ
    SocketError_NameTooLong, // ENAMETOOLONG
    SocketError_BadFlags, // (EAI_BADFLAGS)
    SocketError_TooManyProcesses, // EPROCLIM
    SocketError_DNSTemporaryError, // EAI_AGAIN
    SocketError_DNSHostNotFound, // EAI_NONAME
    SocketError_DNSUnsupportedServiceName, // EAI_SERVICE
    SocketError_DNSFailure, // EAI_FAIL

    // Windows-specific:
    SocketError_NetworkSystemNotReady, // WSASYSNOTREADY
    SocketError_WSAVersionNotSupported, // WSAVERNOTSUPPORTED
} typedef SocketError;

struct SocketLingerOptions
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

// flags for SocketDNSRequest/SocketDNSResponse.
typedef unsigned char SocketGetAddrInfoFlags;
#define SOCKET_AI_NOFLAGS 0
#define SOCKET_AI_FLAG_PASSIVE 1
#define SOCKET_AI_FLAG_CANONNAME 2
#define SOCKET_AI_FLAG_NUMERICHOST 4
#define SOCKET_AI_FLAG_NUMERICSERV 8
#define SOCKET_AI_FLAG_ADDRCONFIG 16
#define SOCKET_AI_FLAG_IPV4MAPPED 32
#define SOCKET_AI_FLAG_BOTHIPVERS 64

// flags for socket_getnameinfo.
typedef unsigned char SocketGetNameInfoFlags;
#define SOCKET_NI_NOFLAGS 0
#define SOCKET_NI_FLAG_NOFQDN 1
#define SOCKET_NI_FLAG_NUMERICHOST 2
#define SOCKET_NI_FLAG_NUMERICSERV 4
#define SOCKET_NI_FLAG_DGRAM 8
#define SOCKET_NI_FLAG_NAMEREQD 16

// flags & type definitions for socket_shutdown.
typedef unsigned char SocketShutdownFlags;
#define SOCKET_SD_FLAG_RECV 1 // 01b
#define SOCKET_SD_FLAG_SEND 2 // 10b

typedef struct in_addr IPv4Address;
typedef struct in6_addr IPv6Address;
typedef void IPAddressInterface;

#define IPV4ADDRSTRSIZE INET_ADDRSTRLEN
#define IPV6ADDRSTRSIZE INET6_ADDRSTRLEN

#define IPV4ADDR_INIT(addr) (IPv4Address){ .s_addr = SOCKET_HTONL(addr) }
#define IPV4ADDR_PACK(a, b, c, d) ((((uint32_t)(a) & 0xFF) << 24) | (((uint32_t)(b) & 0xFF) << 16) | (((uint32_t)(c) & 0xFF) << 8) | ((uint32_t)(d) & 0xFF))

LIBSOCKET_API extern const IPv4Address IPV4ADDR_ANY;
LIBSOCKET_API extern const IPv4Address IPV4ADDR_LOOPBACK;
LIBSOCKET_API extern const IPv4Address IPV4ADDR_BROADCAST;

LIBSOCKET_API extern const IPv6Address IPV6ADDR_ANY;
LIBSOCKET_API extern const IPv6Address IPV6ADDR_LOOPBACK;

typedef struct Socket Socket;

typedef void SocketAddressInterface;
typedef struct sockaddr SocketAddressBase;

#define SOCKET_GETSOCKADDRAF(sockaddr_ptr) ((SocketAddressFamily)(((const SocketAddressBase *)(sockaddr_ptr))->sa_family))

typedef void SocketIPAddressInterface;
typedef struct sockaddr_in SocketIPv4Address;
typedef struct sockaddr_in6 SocketIPv6Address;

typedef struct sockaddr_storage SocketAddressStorage;

#define LIBSOCKET_SOCKETDNSBASE \
    SocketGetAddrInfoFlags flags; /* see SOCKET_AI_... flags for more info. */\
    SocketAddressFamily af;\
    SocketType type;\
    SocketProtocol protocol;

struct SocketDNSRequest
{
    LIBSOCKET_SOCKETDNSBASE
} typedef SocketDNSRequest;

struct SocketDNSResponse
{
    LIBSOCKET_SOCKETDNSBASE

    SocketAddressInterface *sockaddr;
    size_t sockaddrlen;
    char *canonname;

    struct SocketDNSResponse *next;
} typedef SocketDNSResponse;

#undef LIBSOCKET_SOCKETDNSBASE

// memory allocators must work same as in <stdlib.h>.
struct LibSocketAllocators
{
    void *(*malloc)(size_t);
    void *(*realloc)(void *, size_t); // must be safe for NULL.
    void (*free)(void *); // must be safe for NULL.
} typedef LibSocketAllocators;

struct LibSocketPanicInfo
{
    // general info.
    const char *file;
    long long line;
    const char *function;
    const char *description;

    SocketError error; // optional (equals SocketError_Success if true value not present).
} typedef LibSocketPanicInfo;

typedef void LibSocketPanicHandler(const LibSocketPanicInfo *);
//typedef void LibSocketDebugInfoHandler(const LibSocket)

struct SocketStartupOptions
{
    const LibSocketAllocators *allocators; // can be NULL.
    LibSocketPanicHandler *panichandler; // can be NULL.
    
    unsigned short winsock_version;
} typedef SocketStartupOptions;

#define SOCKSTUPOPTS_DEFAULTINIT \
    {\
        .allocators = NULL,\
        .panichandler = NULL,\
        .winsock_version = LIBSOCKET_WINSOCK_DEFAULT_VERSION\
    }

LIBSOCKET_API const char * LIBSOCKET_ABI socket_strerror(SocketError errcode); // can be accessed without library initialization.

LIBSOCKET_API bool LIBSOCKET_ABI libsocket_initialized(void); // can be accessed without library initialization.
LIBSOCKET_API SocketError LIBSOCKET_ABI libsocket_startup(const SocketStartupOptions *options); // options can be NULL.
LIBSOCKET_API SocketError LIBSOCKET_ABI libsocket_cleanup(void);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_parseipaddr(IPAddressInterface *addr, SocketAddressFamily af, const char *straddr);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_ipaddrtostr(const IPAddressInterface *addr, SocketAddressFamily af, char *straddr, socklen_t size);

// [socket_packsockipaddr]: can be accessed without library initialization.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_packsockipaddr(SocketIPAddressInterface *sockaddr, SocketAddressFamily af, const IPAddressInterface *addr, unsigned short port);
// [socket_unpacksockipaddr]: can be accessed without library initialization.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_unpacksockipaddr(const SocketIPAddressInterface *sockaddr, SocketAddressFamily af, IPAddressInterface *addr, unsigned short *port);

// [socket_getaddrinfo]: request can be NULL, and host OR service also can be NULL, but not both. see <netdb.h> getaddrinfo function documentation for more info.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getaddrinfo(const char *hostname, const char *servicename, const SocketDNSRequest *request, SocketDNSResponse **response);
LIBSOCKET_API void LIBSOCKET_ABI socket_freeaddrinfo(SocketDNSResponse *response); // safe for NULL pointer and can be accessed without library initialization.
// [socket_getnameinfo]: hostname & servicename can be NULL, and hostnamesize OR servicenamesize can be NULL, but not both. (see <netdb.h> getnameinfo function documentation for more info.)
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getnameinfo(const SocketIPAddressInterface *sockaddr, socklen_t sockaddrlen, char *hostname, size_t *hostnamesize, char *servicename, size_t *servicenamesize, SocketGetNameInfoFlags flags);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_open(Socket **socket, SocketAddressFamily af, SocketType type, SocketProtocol protocol);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_close(Socket *socket);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_connect(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_bind(const Socket *socket, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_listen(const Socket *socket, int backlog);
// [socket_accept]: sockaddr & sockaddrlen can be NULL. see <sys/socket.h> accept function documentation for more info.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_accept(Socket **acceptedsocket, const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen);

// [socket_recv - ... - socket_sendto]: processedbytes can be NULL.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_recv(const Socket *socket, void *buffer, size_t size, size_t *processedbytes, int flags);
// [socket_recvfrom]: sockaddr & sockaddrlen can be NULL. see <sys/socket.h> recvfrom function documentation for more info.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_recvfrom(const Socket *socket, void *buffer, size_t size, size_t *processedbytes, int flags, SocketAddressInterface *sockaddr, socklen_t *sockaddrlen);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_send(const Socket *socket, const void *data, size_t size, size_t *processedbytes, int flags);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_sendto(const Socket *socket, const void *buffer, size_t size, size_t *processedbytes, int flags, const SocketAddressInterface *sockaddr, socklen_t sockaddrlen);

LIBSOCKET_API bool LIBSOCKET_ABI socket_isnonblocking(const Socket *socket); // can be accessed without library initialization.
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_setnonblocking(Socket *socket, bool enable);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getreadablebytes(const Socket *socket, size_t *availbytes);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_shutdown(const Socket *socket, SocketShutdownFlags flags);

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, void *optval, size_t *optsize);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_setopt(const Socket *socket, SocketOptionLevel level, SocketOptionName optname, const void *optval, size_t optsize);

LIBSOCKET_API SocketAddressFamily LIBSOCKET_ABI socket_getaf(const Socket *socket); // can be accessed without library initialization.
LIBSOCKET_API SocketType LIBSOCKET_ABI socket_gettype(const Socket *socket); // can be accessed without library initialization.
LIBSOCKET_API SocketProtocol LIBSOCKET_ABI socket_getprotocol(const Socket *socket); // can be accessed without library initialization.

LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getpeername(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size);
LIBSOCKET_API SocketError LIBSOCKET_ABI socket_getsockname(const Socket *socket, SocketAddressInterface *sockaddr, socklen_t *size);

#if defined(LIBSOCKET_ALLOWUNSAFEACCESS) || defined(LIBSOCKET_EXPORT)
    LIBSOCKET_API SOCKETDESCRIPTOR LIBSOCKET_ABI socket_gethandle(const Socket *socket); // can be accessed without library initialization.
#endif

#ifdef __cplusplus
    }
#endif

#endif
