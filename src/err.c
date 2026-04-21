#include "libsocket.h"
#include "err.h"

#include "init.h"

SocketError socket_getlasterror(void)
{
    ENSURE_INIT;

    switch (GETLASTERROR())
    {
        case SOCKERR_NOMEM:
            return MemoryAllocationFailed;

        case SOCKERR_PARSEADDRFAIL:
            return ParsingAddressFailed;

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

        case SOCKERR_INVDESC:
            return InvalidDescriptor;

	    case SOCKERR_INPROGRESS:
	        return OperationInProgress;

        case SOCKERR_NOSPC:
            return NoSpaceLeft;

	    #if SOCKERR_AGAIN != SOCKERR_WOULDBLOCK
		    case SOCKERR_AGAIN:
	    #endif
        case SOCKERR_WOULDBLOCK:
            return TemporaryUnavailable;

        #ifdef OS_WINDOWS
            case WSANOTINITIALISED:
                return InitializationError;
        #endif

        default:
            return InternalUnknownError;
    }
}
