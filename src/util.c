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

const char *LIBSOCKET_MODULENAME = "libsocket";

const IPv4Address IPV4ADDR_ANY = IPV4ADDR_INIT(INADDR_ANY);
const IPv4Address IPV4ADDR_LOOPBACK = IPV4ADDR_INIT(INADDR_LOOPBACK);
const IPv4Address IPV4ADDR_BROADCAST = IPV4ADDR_INIT(INADDR_BROADCAST);

const IPv6Address IPV6ADDR_ANY = IN6ADDR_ANY_INIT;
const IPv6Address IPV6ADDR_LOOPBACK = IN6ADDR_LOOPBACK_INIT;

NMemoryAllocators __libsocket_allocators = {0};
NPanicHandler *__libsocket_panichandler = NULL;
NAlertHandler *__libsocket_alerthandler = NULL;
NUnorderedSet *__libsocket_sockslist = NULL;
NThreadMutex *__libsocket_sockslistmutex = NULL;

NError __libsocket_closesocket(Socket *socket)
{
    if (CLOSESOCKETDESC(socket->desc)) return GETLASTTRANSLATEDSYSERR();

    if (mutex_destroy(socket->mutex_nonblocking) != MUTEXERROR_SUCCESS)
    { panic_general(NError_MutexAPIError, "Unable to destroy mutex after closing socket descriptor."); }

    allocs.free(socket);
    return NError_Success;
}

void __libsocket_defaultpanichandler(const char *module, const char *file, long long line, const char *function, const char *description, NError error)
{
    fprintf(stderr, "\n\n\n###################\n# LIBSOCKET PANIC #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n", file, line, function);

    if (error != PANIC_NOERRORCODE) fprintf(stderr, "    \"%s\" because\n    ", ncore_strerror(error));

    fprintf(stderr, "    %s\n\n###################\n\n", description);
}

void __libsocket_defaultalerthandler(const char *module, const char *file, long long line, const char *function, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\n\n\n###################\n# LIBSOCKET ALERT #\n###################\n\n");

    fprintf(stderr, "In \"%s\" at line %lld (%s):\n    ", file, line, function);
    vfprintf(stderr, format, args);

    fputs("\n\n###################\n", stderr);
    va_end(args);
}