#include <stdio.h>

#define LIBSOCKET_ALLOWUNSAFEACCESS
#include "libsocket.h"

int main(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) { puts("socket_open error. Aborted."); return 1; }

    printf("Socket descriptor: %i\n", socket_gethandle(s));

    if (!socket_close(s)) { puts("socket_close error. Aborted."); return 1; }

    return 0;
}