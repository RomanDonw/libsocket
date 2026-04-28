#include "libsocket.h"

#include <stddef.h>

const char *socket_strerror(SocketError errcode)
{
    switch (errcode)
    {
        case InternalUnknownError:
            return "internal unknown socket error";

        case MemoryAllocationFailed:
            return "memory allocation failed";

        case ParsingAddressFailed:
            return "parsing address failed";

        case Interrupted:
            return "socket operation interrupted";

        case AccessDenied:
            return "access denied";

        case Fault:
            return "fault";
        
        case IncorrectArgumentValue:
            return "incorrect argument value";

        case TooManyOpenedSockets:
            return "too many opened sockets";

        case TemporaryUnavailable:
            return "temporary unavailable";

        case OperationInProgress:
            return "operation in progress";

        case InExecutionProcess:
            return "processing task";

        case UnsupportedAddressFamily:
            return "unsupported address family";

        case UnsupportedProtocol:
            return "unsupported network protocol";

        case UnsupportedSocketType:
            return "unsupported socket type";

        case AddressInUse:
            return "address already in use";

        case NetworkUnreachable:
            return "network unreachable";

        case NetworkDown:
            return "network down";

        case NetworkReset:
            return "network reset";

        case ConnectionReset:
            return "connection reset";

        case ConnectionRefused:
            return "connection refused";

        case ConnectionTimedOut:
            return "connection timed out";

        case NotConnected:
            return "not connected";

        case InvalidDescriptor:
            return "invalid descriptor";

        case NoSpaceLeft:
            return "no space left";

        case ProtocolOptionUnsupported:
            return "protocol option unsupported";

        case OperationNotSupported:
            return "operation not supported";

        case SystemBufferOverflowed:
            return "system buffer overflowed";

        case ConnectionAborted:
            return "connection aborted";

        // Windows-specific:
        case InitializationError:
            return "WinSock not initialized";

        default:
            return NULL;
    }
}
