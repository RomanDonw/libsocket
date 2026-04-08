#include "libsocket.h"

#include <stdlib.h>

#ifdef OS_WINDOWS
    void __attribute__((constructor(101))) init()
    {
        const WORD version = MAKEWORD(2, 2);

        WSADATA data;
        if (WSAStartup(version, &data)) abort();

        if (data.wVersion != version) { WSACleanup(); abort(); }
    }

    void __attribute__((destructor)) cleanup()
    {
        WSACleanup();
    }
#else
    #include <unistd.h>
#endif

Socket *socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    Socket *ret = malloc(sizeof(Socket));
    if (!ret) return NULL;

    ret->af = af;
    ret->type = type;
    ret->protocol = protocol;

    SOCKETDESCRIPTOR desc;
    if ((desc = socket(af, type, protocol)) == InvalidSocket)
    {
        free(ret);
        return NULL;
    }

    ret->desc = desc;

    return ret;
}

bool socket_close(Socket *socket)
{
    #ifdef OS_WINDOWS
        if (closesocket(socket->desc)) return false;
    #else
        if (close(socket->desc)) return false;
    #endif

    free(socket);

    return true;
}

bool socket_listen(Socket *socket, int backlog) { return !listen(socket->desc, backlog); }