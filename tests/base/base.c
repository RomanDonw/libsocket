/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "base.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef LIBSOCKET_OS_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
    #include <time.h>
#endif

int main(void)
{
    printf(" === TEST \"%s\" STARTED ===\n\n", testname);

    SocketError err = libsocket_startup(NULL, NULL);
    if (err != SocketError_Success) handlesockerror(err, "libsocket_startup");

    test();

    err = libsocket_cleanup();
    if (err != SocketError_Success) handlesockerror(err, "libsocket_cleanup");

    printf("\n === TEST \"%s\" PASSED ===\n", testname);
    return 0;
}

void testabort_f(char *reason)
{
    printf("\n === TEST \"%s\" FAILED ===\n", testname);
    if (reason) { printf("Reason: \"%s\"\n", reason); free(reason); }
    exit(0);
}

void testabort_c(const char *reason)
{
    printf("\n === TEST \"%s\" FAILED ===\n", testname);
    if (reason) printf("Reason: \"%s\"\n", reason);
    exit(0);
}

void handlesockerror(SocketError err, const char *funcname)
{
    #define FMTLASTERR(buff, size) snprintf(buff, size, "%s error: %s.\n", funcname, socket_strerror(err))

    int len = FMTLASTERR(NULL, 0);
    if (len <= 0) testabort_c("handlesockerr internal error 1");

    char *msg = malloc_s(len);
    if (FMTLASTERR(msg, len) < 0) { free(msg); testabort_c("handlesockerr internal error 2"); }

    testabort_f(msg);

    #undef FMTLASTERR
}

const char *memallocerrorstr = "Memory (re)allocation failed. Application aborted.";

void *malloc_s(size_t size)
{
    void *ret = malloc(size);
    if (!ret) testabort_c(memallocerrorstr);
    return ret;
}

void *realloc_s(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (!ret) testabort_c(memallocerrorstr);
    return ret;
}

void waitms(uint32_t milliseconds)
{
    #ifdef LIBSOCKET_OS_WINDOWS
        Sleep(milliseconds);
    #else
        struct timespec d;
        d.tv_sec = milliseconds / 1000;
        d.tv_nsec = (milliseconds % 1000) * 1000000;
        nanosleep(&d, NULL);
    #endif
}