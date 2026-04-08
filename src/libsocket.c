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

Socket socket_open(SocketAddressFamily af, SocketType type, SocketProtocol protocol)
{
    return socket(af, type, protocol);
}

bool socket_close(Socket socket)
{
    #ifdef OS_WINDOWS
        return !closesocket(socket);
    #else
        return !close(socket);
    #endif
}