#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include "libsocket.h"

void handleerror(const char *funcname)
{
    printf("%s error: %s. Application aborted.\n", funcname, socket_strerror(socket_getlasterror()));
    abort();
}

#endif