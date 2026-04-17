#include "libsocket.h"

#include "../init.h"

#include <stddef.h>

const char *socket_strerror(SocketError errcode)
{
    ENSURE_INIT;

    switch (errcode)
    {
        case InternalUnknownError:
            return "internal unknown socket error";

        case Interrupted:
            return "socket operation interrupted";

        case AccessDenied:
            return "access denied";

        case InvalidAddress:
            return "invalid or incorrect address";
        
        case IncorrectArgumentValue:
            return "incorrect argument value";

        case TooManyOpenedSockets:
            return "too many opened sockets";

        case TemporaryUnavailable:
            return "temporary unavailable";

        case InExecutionProcess:
            return "processing task";

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

        case ConnectionReset:
            return "connection reset";

        case ConnectionRefused:
            return "connection refused";

        case ConnectionTimedOut:
            return "connection timed out";

        case InitializationError:
            return "WinSock not initialized";

        default:
            return NULL;
    }
}