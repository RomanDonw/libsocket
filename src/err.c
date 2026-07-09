/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"
#include "err.h"
#include "util.h"

NError __libsocket_translateerror(int err)
{
    switch (err)
    {
        case 0:
            return NError_Success;

        case SOCKERR_NOMEM:
            return NError_MemoryAllocationFailed;

        case SOCKERR_INTR:
            return NError_Interrupted;

        case SOCKERR_ACCES:
            return NError_AccessDenied;

        case SOCKERR_FAULT:
            return NError_Fault;

        case SOCKERR_INVAL:
            return NError_IncorrectArgumentValue;

        case SOCKERR_MFILE:
            return NError_TooManyOpenedDescriptors;

        case SOCKERR_ALREADY:
            return NError_InExecutionProcess;

        case SOCKERR_AFNOSUPPORT:
            return NError_UnsupportedAddressFamily;

        case SOCKERR_PROTONOSUPPORT:
            return NError_UnsupportedProtocol;

        case SOCKERR_SOCKTNOSUPPORT:
            return NError_UnsupportedSocketType;

        case SOCKERR_ADDRINUSE:
            return NError_AddressInUse;

        case SOCKERR_ADDRNOTAVAIL:
            return NError_AddressNotAvailable;

        case SOCKERR_NETUNREACH:
            return NError_NetworkUnreachable;
        
        case SOCKERR_NETDOWN:
            return NError_NetworkDown;

        case SOCKERR_NETRESET:
            return NError_NetworkReset;

        case SOCKERR_CONNRESET:
            return NError_ConnectionReset;

        case SOCKERR_CONNREFUSED:
            return NError_ConnectionRefused;

        case SOCKERR_TIMEDOUT:
            return NError_ConnectionTimedOut;

        case SOCKERR_NOTCONN:
            return NError_NotConnected;

        case SOCKERR_NOTSOCK:
        case SOCKERR_INVDESC:
            return NError_InvalidDescriptor;

	    case SOCKERR_INPROGRESS:
	        return NError_OperationInProgress;

        case SOCKERR_NOPROTOOPT:
            return NError_UnsupportedProtocolOption;

        case SOCKERR_OPNOTSUPP:
            return NError_UnsupportedOperation;

        case SOCKERR_NOBUFFS:
            return NError_SystemBufferOverflowed;

        case SOCKERR_CONNABORTED:
            return NError_ConnectionAborted;

        case SOCKERR_LOOP:
            return NError_CannotTranslateName;

        case SOCKERR_DESTADDRREQ:
            return NError_DestinationAddressRequired;

        case SOCKERR_ISCONN:
            return NError_AlreadyConnected;

        case SOCKERR_NAMETOOLONG:
            return NError_NameTooLong;

        #ifdef SOCKERR_PROCLIM
            case SOCKERR_PROCLIM:
                return NError_TooManyProcesses;
        #endif

        #ifdef LIBSOCKET_OS_WINDOWS
            case SOCKERR_WOULDBLOCK:
                return NError_WouldBlock;

            case WSANOTINITIALISED:
                return NError_NotInitialized;

            case WSASYSNOTREADY:
                return NError_NetworkSystemNotReady;

            case WSAVERNOTSUPPORTED:
                return NError_WSAVersionNotSupported;
        #else
            #if EAGAIN != SOCKERR_WOULDBLOCK
                case EAGAIN:
            #endif
            case SOCKERR_WOULDBLOCK:
                return NError_WouldBlock;

            case ENOSPC:
                return NError_NoSpaceLeft;
        #endif

        default:
            alert("Got unhandled system error: %i.", err);
            return NError_InternalUnknownError;
    }
}
