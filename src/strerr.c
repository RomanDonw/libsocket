#include "libsocket.h"

#include <stddef.h>

const char *socket_strerror(SocketError errcode)
{
    switch (errcode)
    {
        case SocketError_Success:
            return "success (no error)";

        case SocketError_NotInitialized:
            return "not initialized";

        case SocketError_AlreadyInitialized:
            return "already initialized";

        case SocketError_InitializationError:
            return "initialization error";

        case SocketError_InternalUnknownError:
            return "internal unknown socket error";

        case SocketError_MemoryAllocationFailed:
            return "memory allocation failed";

        case SocketError_ParsingAddressFailed:
            return "parsing address failed";

        case SocketError_Interrupted:
            return "socket operation interrupted";

        case SocketError_AccessDenied:
            return "access denied";

        case SocketError_Fault:
            return "fault";

        case SocketError_InsufficientBufferSize:
            return "insufficient buffer size";
        
        case SocketError_IncorrectArgumentValue:
            return "incorrect argument value";

        case SocketError_TooManyOpenedSockets:
            return "too many opened sockets";

        case SocketError_WouldBlock:
            return "would block";

        case SocketError_OperationInProgress:
            return "operation in progress";

        case SocketError_InExecutionProcess:
            return "processing task";

        case SocketError_UnsupportedAddressFamily:
            return "unsupported address family";

        case SocketError_UnsupportedProtocol:
            return "unsupported network protocol";

        case SocketError_UnsupportedSocketType:
            return "unsupported socket type";

        case SocketError_AddressInUse:
            return "address already in use";

        case SocketError_AddressNotAvailable:
            return "address not available";

        case SocketError_NetworkUnreachable:
            return "network unreachable";

        case SocketError_NetworkDown:
            return "network down";

        case SocketError_NetworkReset:
            return "network reset";

        case SocketError_ConnectionReset:
            return "connection reset";

        case SocketError_ConnectionRefused:
            return "connection refused";

        case SocketError_ConnectionTimedOut:
            return "connection timed out";

        case SocketError_NotConnected:
            return "not connected";

        case SocketError_InvalidDescriptor:
            return "invalid descriptor";

        case SocketError_NoSpaceLeft:
            return "no space left";

        case SocketError_ProtocolOptionUnsupported:
            return "protocol option unsupported";

        case SocketError_OperationNotSupported:
            return "operation not supported";

        case SocketError_SystemBufferOverflowed:
            return "system buffer overflowed";

        case SocketError_ConnectionAborted:
            return "connection aborted";

        case SocketError_CannotTranslateName:
            return "cannot translate name";

        case SocketError_DestinationAddressRequired:
            return "destination address required";

        case SocketError_AlreadyConnected:
            return "already connected";

        case SocketError_NameTooLong:
            return "name too long";

        case SocketError_TooManyProcesses:
            return "too many processes";

        case SocketError_NetworkSystemNotReady:
            return "network system not ready";

        case SocketError_WSAVersionNotSupported:
            return "WinSock version not supported";

        case SocketError_WSAVersionsNotMatch:
            return "responced WinSock version doesn't match requested version";

        case SocketError_DNSFailure:
            return "DNS failure";

        case SocketError_DNSHostNotFound:
            return "(DNS) host not found";

        case SocketError_DNSTemporaryError:
            return "DNS temporary error";

        case SocketError_DNSUnsupportedServiceName:
            return "(DNS) unsupported service name";

        default:
            return NULL;
    }
}
