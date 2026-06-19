/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

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

        case SocketError_InternalUnknownError:
            return "internal unknown socket error (this is a bug, report about it after debugging)";

        case SocketError_InternalSizeMismatch:
            return "internal size mismatch (this is a bug, report about it after debugging)";
            
        case SocketError_InternalVariableOverflow:
            return "internal variable overflow (this is a bug, report about it after debugging)";

        case SocketError_MemoryAllocationFailed:
            return "memory allocation failed";

        case SocketError_ParsingAddressFailed:
            return "parsing address failed";

        case SocketError_MutexAPIError:
            return "mutex API error";

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

        case SocketError_UnsupportedProtocolOption:
            return "unsupported protocol option";

        case SocketError_UnsupportedOperation:
            return "unsupported operation";

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

        case SocketError_BadFlags:
            return "bad flags";

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
