#include "libsocket.h"
#include "err.h"

/*
#define CUSTOMERR_STARTOFFSET -1024
#define CUSTOMERR_STEP -1
#define CUSTOMERR(index) ((CUSTOMERR_STARTOFFSET) + (CUSTOMERR_STEP) * index)
*/

#ifdef LIBSOCKET_OS_WINDOWS
    #define SOCKERR_INTR WSAEINTR
    #define SOCKERR_ACCES WSAEACCES
    #define SOCKERR_FAULT WSAEFAULT
    #define SOCKERR_INVAL WSAEINVAL
    #define SOCKERR_MFILE WSAEMFILE
    #define SOCKERR_INPROGRESS WSAEINPROGRESS
    #define SOCKERR_ALREADY WSAEALREADY
    #define SOCKERR_INVDESC WSA_INVALID_HANDLE
    #define SOCKERR_NOTSOCK WSAENOTSOCK
    #define SOCKERR_OPNOTSUPP WSAEOPNOTSUPP
    #define SOCKERR_NOBUFFS WSAENOBUFS
    #define SOCKERR_LOOP WSAELOOP
    #define SOCKERR_NAMETOOLONG WSAENAMETOOLONG
    #define SOCKERR_NOMEM WSA_NOT_ENOUGH_MEMORY

    #define SOCKERR_WOULDBLOCK WSAEWOULDBLOCK

    #define SOCKERR_ADDRINUSE WSAEADDRINUSE
    #define SOCKERR_ADDRNOTAVAIL WSAEADDRNOTAVAIL
    #define SOCKERR_DESTADDRREQ WSAEDESTADDRREQ

    #define SOCKERR_AFNOSUPPORT WSAEAFNOSUPPORT
    #define SOCKERR_PROTONOSUPPORT WSAEPROTONOSUPPORT
    #define SOCKERR_SOCKTNOSUPPORT WSAESOCKTNOSUPPORT
    #define SOCKERR_NOPROTOOPT WSAENOPROTOOPT

    #define SOCKERR_NETUNREACH WSAENETUNREACH
    #define SOCKERR_NETDOWN WSAENETDOWN
    #define SOCKERR_NETRESET WSAENETRESET

    #define SOCKERR_CONNRESET WSAECONNRESET
    #define SOCKERR_CONNREFUSED WSAECONNREFUSED
    #define SOCKERR_CONNABORTED WSAECONNABORTED
    #define SOCKERR_TIMEDOUT WSAETIMEDOUT
    #define SOCKERR_NOTCONN WSAENOTCONN
    #define SOCKERR_ISCONN WSAEISCONN
#else
    #define SOCKERR_INTR EINTR
    #define SOCKERR_ACCES EACCES
    #define SOCKERR_FAULT EFAULT
    #define SOCKERR_INVAL EINVAL
    #define SOCKERR_MFILE EMFILE
    #define SOCKERR_INPROGRESS EINPROGRESS
    #define SOCKERR_ALREADY EALREADY
    #define SOCKERR_INVDESC EBADF
    #define SOCKERR_NOSPC ENOSPC
    #define SOCKERR_NOTSOCK ENOTSOCK
    #define SOCKERR_OPNOTSUPP EOPNOTSUPP
    #define SOCKERR_NOBUFFS ENOBUFS
    #define SOCKERR_LOOP ELOOP
    #define SOCKERR_NAMETOOLONG ENAMETOOLONG
    #define SOCKERR_NOMEM ENOMEM

    #define SOCKERR_WOULDBLOCK EWOULDBLOCK

    #define SOCKERR_ADDRINUSE EADDRINUSE
    #define SOCKERR_ADDRNOTAVAIL EADDRNOTAVAIL
    #define SOCKERR_DESTADDRREQ EDESTADDRREQ

    #define SOCKERR_AFNOSUPPORT EAFNOSUPPORT
    #define SOCKERR_PROTONOSUPPORT EPROTONOSUPPORT
    #define SOCKERR_SOCKTNOSUPPORT ESOCKTNOSUPPORT
    #define SOCKERR_NOPROTOOPT ENOPROTOOPT

    #define SOCKERR_NETUNREACH ENETUNREACH
    #define SOCKERR_NETDOWN ENETDOWN
    #define SOCKERR_NETRESET ENETRESET

    #define SOCKERR_CONNRESET ECONNRESET
    #define SOCKERR_CONNREFUSED ECONNREFUSED
    #define SOCKERR_CONNABORTED ECONNABORTED
    #define SOCKERR_TIMEDOUT ETIMEDOUT
    #define SOCKERR_NOTCONN ENOTCONN
    #define SOCKERR_ISCONN EISCONN
#endif

SocketError socket_lasterror = Success;

SocketError translateerror(int err)
{
    switch (err)
    {
        case SOCKERR_NOMEM:
            return MemoryAllocationFailed;

        case SOCKERR_INTR:
            return Interrupted;

        case SOCKERR_ACCES:
            return AccessDenied;

        case SOCKERR_FAULT:
            return Fault;

        case SOCKERR_INVAL:
            return IncorrectArgumentValue;

        case SOCKERR_MFILE:
            return TooManyOpenedSockets;

        case SOCKERR_ALREADY:
            return InExecutionProcess;

        case SOCKERR_AFNOSUPPORT:
            return UnsupportedAddressFamily;

        case SOCKERR_PROTONOSUPPORT:
            return UnsupportedProtocol;

        case SOCKERR_SOCKTNOSUPPORT:
            return UnsupportedSocketType;

        case SOCKERR_ADDRINUSE:
            return AddressInUse;

        case SOCKERR_ADDRNOTAVAIL:
            return AddressNotAvailable;

        case SOCKERR_NETUNREACH:
            return NetworkUnreachable;
        
        case SOCKERR_NETDOWN:
            return NetworkDown;

        case SOCKERR_NETRESET:
            return NetworkReset;

        case SOCKERR_CONNRESET:
            return ConnectionReset;

        case SOCKERR_CONNREFUSED:
            return ConnectionRefused;

        case SOCKERR_TIMEDOUT:
            return ConnectionTimedOut;

        case SOCKERR_NOTCONN:
            return NotConnected;

        case SOCKERR_NOTSOCK:
        case SOCKERR_INVDESC:
            return InvalidDescriptor;

	    case SOCKERR_INPROGRESS:
	        return OperationInProgress;

        case SOCKERR_NOPROTOOPT:
            return ProtocolOptionUnsupported;

        case SOCKERR_OPNOTSUPP:
            return OperationNotSupported;

        case SOCKERR_NOBUFFS:
            return SystemBufferOverflowed;

        case SOCKERR_CONNABORTED:
            return ConnectionAborted;

        case SOCKERR_LOOP:
            return CannotTranslateName;

        case SOCKERR_DESTADDRREQ:
            return DestinationAddressRequired;

        case SOCKERR_ISCONN:
            return AlreadyConnected;

        case SOCKERR_NAMETOOLONG:
            return NameTooLong;

        #ifdef LIBSOCKET_OS_WINDOWS
            case SOCKERR_WOULDBLOCK:
                return WouldBlock;

            case WSANOTINITIALISED:
                return NotInitialized;
        #else
            #if EAGAIN != SOCKERR_WOULDBLOCK
                case EAGAIN:
            #endif
            case SOCKERR_WOULDBLOCK:
                return WouldBlock;

            case ENOSPC:
                return NoSpaceLeft;
        #endif

        default:
            return InternalUnknownError;
    }
}
