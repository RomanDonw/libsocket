#include <stdio.h>

#define LIBSOCKET_ALLOWUNSAFEACCESS
#include "libsocket.h"
#include "util.h"

int main(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) handleerror("socket_open");

    printf("Socket descriptor: %i\n", socket_gethandle(s));

    if (!socket_close(s)) handleerror("socket_close");

    return 0;
}