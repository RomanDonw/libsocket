#include <stdio.h>
#include <stdlib.h>
#include "libsocket.h"

int main(void)
{
    Socket *s = socket_open(IPv4, Stream, TCP);
    if (!s) { puts("socket_open error"); abort(); }

    if (!socket_close(s)) { puts("socket_close error"); abort(); }

    return 0;
}