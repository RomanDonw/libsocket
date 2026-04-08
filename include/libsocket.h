#ifndef LIBSOCKET_H
#define LIBSOCKET_H

#ifdef __cplusplus
    extern "C"
    {
#endif

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
    #define OS_WINDOWS
#endif

#ifdef OS_WINDOWS
    #include <winsock2.h>

    typedef SOCKET Socket;
    #define InvalidSocket INVALID_SOCKET;
#else
    #include <sys/socket.h>

    typedef int Socket;
    #define InvalidSocket -1
#endif



#ifdef __cplusplus
    }
#endif

#endif