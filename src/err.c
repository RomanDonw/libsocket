#include "libsocket.h"
#include "err.h"

#ifdef LIBSOCKET_DEBUG
    #include <stdio.h>
#endif

SocketError socket_lasterror = SocketError_Success;

SocketError translateerror(int err)
{
    switch (err)
    {
        case 0:
            return SocketError_Success;

        case SOCKERR_NOMEM:
            return SocketError_MemoryAllocationFailed;

        case SOCKERR_INTR:
            return SocketError_Interrupted;

        case SOCKERR_ACCES:
            return SocketError_AccessDenied;

        case SOCKERR_FAULT:
            return SocketError_Fault;

        case SOCKERR_INVAL:
            return SocketError_IncorrectArgumentValue;

        case SOCKERR_MFILE:
            return SocketError_TooManyOpenedSockets;

        case SOCKERR_ALREADY:
            return SocketError_InExecutionProcess;

        case SOCKERR_AFNOSUPPORT:
            return SocketError_UnsupportedAddressFamily;

        case SOCKERR_PROTONOSUPPORT:
            return SocketError_UnsupportedProtocol;

        case SOCKERR_SOCKTNOSUPPORT:
            return SocketError_UnsupportedSocketType;

        case SOCKERR_ADDRINUSE:
            return SocketError_AddressInUse;

        case SOCKERR_ADDRNOTAVAIL:
            return SocketError_AddressNotAvailable;

        case SOCKERR_NETUNREACH:
            return SocketError_NetworkUnreachable;
        
        case SOCKERR_NETDOWN:
            return SocketError_NetworkDown;

        case SOCKERR_NETRESET:
            return SocketError_NetworkReset;

        case SOCKERR_CONNRESET:
            return SocketError_ConnectionReset;

        case SOCKERR_CONNREFUSED:
            return SocketError_ConnectionRefused;

        case SOCKERR_TIMEDOUT:
            return SocketError_ConnectionTimedOut;

        case SOCKERR_NOTCONN:
            return SocketError_NotConnected;

        case SOCKERR_NOTSOCK:
        case SOCKERR_INVDESC:
            return SocketError_InvalidDescriptor;

	    case SOCKERR_INPROGRESS:
	        return SocketError_OperationInProgress;

        case SOCKERR_NOPROTOOPT:
            return SocketError_ProtocolOptionUnsupported;

        case SOCKERR_OPNOTSUPP:
            return SocketError_OperationNotSupported;

        case SOCKERR_NOBUFFS:
            return SocketError_SystemBufferOverflowed;

        case SOCKERR_CONNABORTED:
            return SocketError_ConnectionAborted;

        case SOCKERR_LOOP:
            return SocketError_CannotTranslateName;

        case SOCKERR_DESTADDRREQ:
            return SocketError_DestinationAddressRequired;

        case SOCKERR_ISCONN:
            return SocketError_AlreadyConnected;

        case SOCKERR_NAMETOOLONG:
            return SocketError_NameTooLong;

        #ifdef SOCKERR_PROCLIM
            case SOCKERR_PROCLIM:
                return SocketError_TooManyProcesses;
        #endif

        #ifdef LIBSOCKET_OS_WINDOWS
            case SOCKERR_WOULDBLOCK:
                return SocketError_WouldBlock;

            case WSANOTINITIALISED:
                return SocketError_NotInitialized;

            case WSASYSNOTREADY:
                return SocketError_NetworkSystemNotReady;

            case WSAVERNOTSUPPORTED:
                return SocketError_WSAVersionNotSupported;
        #else
            #if EAGAIN != SOCKERR_WOULDBLOCK
                case EAGAIN:
            #endif
            case SOCKERR_WOULDBLOCK:
                return SocketError_WouldBlock;

            case ENOSPC:
                return SocketError_NoSpaceLeft;
        #endif

        default:
            #ifdef LIBSOCKET_DEBUG
                fprintf(stderr, "Got unhandled system error: %i.\n", err);
            #endif

            return SocketError_InternalUnknownError;
    }
}
