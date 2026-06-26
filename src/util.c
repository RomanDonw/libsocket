/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "err.h"
#include "types.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <unistd.h>
#endif

const IPv4Address IPV4ADDR_ANY = IPV4ADDR_INIT(INADDR_ANY);
const IPv4Address IPV4ADDR_LOOPBACK = IPV4ADDR_INIT(INADDR_LOOPBACK);
const IPv4Address IPV4ADDR_BROADCAST = IPV4ADDR_INIT(INADDR_BROADCAST);

const IPv6Address IPV6ADDR_ANY = IN6ADDR_ANY_INIT;
const IPv6Address IPV6ADDR_LOOPBACK = IN6ADDR_LOOPBACK_INIT;

LibSocketAllocators __libsocket_allocators = {0};
LibSocketPanicHandler *__libsocket_panichandler = NULL;
LibSocketAlertHandler *__libsocket_alerthandler = NULL;

#ifdef LIBSOCKET_DEBUG
    void __libsocket_logdbgerr(const char *msgformat, ...)
    {
        va_list args;
        va_start(args, msgformat);

        fprintf(stderr, "[libsocket]: ");
        vfprintf(stderr, msgformat, args);
        fputs(".", stderr);

        va_end(args);
    }
#endif

SocketError __libsocket_closesocket(Socket *socket)
{
    if (CLOSESOCKETDESC(socket->desc)) return GETLASTTRANSLATEDSYSERR();

    if (mutex_destroy(socket->mutex_nonblocking) != MUTEXERROR_SUCCESS)
    { panic_general(SocketError_MutexAPIError, "Unable to destroy mutex after closing socket descriptor."); }

    allocs.free(socket);
    return SocketError_Success;
}

void __libsocket_defaultpanichandler(const LibSocketPanicInfo *info)
{
    fprintf(stderr, "\n\n\n###################\n# LIBSOCKET PANIC #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n", info->file, info->line, info->function);

    if (info->error != PANIC_NOERRORCODE) fprintf(stderr, "    \"%s\" because\n    ", socket_strerror(info->error));

    fprintf(stderr, "    %s\n\n###################\n\n", info->description);
}

void __libsocket_defaultalerthandler(const char *file, long long line, const char *function, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\n\n\n###################\n# LIBSOCKET ALERT #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n    ", file, line, function);
    vfprintf(stderr, format, args);

    fputs("\n\n###################\n", stderr);
    va_end(args);
}