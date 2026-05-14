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

    if (!socket_startup(NULL)) handlesockerror("socket_startup");
    test();
    if (!socket_cleanup()) handlesockerror("socket_cleanup");

    printf("\n === TEST \"%s\" PASSED ===\n", testname);
    return 0;
}

void testabort(char *reason, bool freereasonfrommem)
{
    printf("\n === TEST \"%s\" FAILED ===\n", testname);

    if (reason)
    {
        printf("Reason: \"%s\"\n", reason);
        if (freereasonfrommem) free(reason);
    }

    exit(0);
}

void handlesockerror(const char *funcname)
{
    #define FMTLASTERR(buff, size) snprintf(buff, size, "%s error: %s.\n", funcname, socket_strerror(socket_lasterror))

    int len = FMTLASTERR(NULL, 0);
    if (len <= 0) testabort("handlesockerr internal error 1", false);

    char *msg = malloc_s(len);
    if (FMTLASTERR(msg, len) < 0) { free(msg); testabort("handlesockerr internal error 2", false); }

    testabort(msg, true);
}

const char *memallocerrorstr = "Memory (re)allocation failed. Application aborted.";

void *malloc_s(size_t size)
{
    void *ret = malloc(size);
    if (!ret) { puts(memallocerrorstr); abort(); }
    return ret;
}

void *realloc_s(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (!ret) { puts(memallocerrorstr); abort(); }
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