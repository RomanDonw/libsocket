#ifndef BASE_H
#define BASE_H

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>
#include <stdbool.h>

#include "libsocket.h"

extern const char *testname;
extern void test(void);

void testabort(char *reason, bool freereasonfrommem); // reason can be NULL.

void waitms(uint32_t milliseconds);
void handlesockerror(const char *funcname);

void *malloc_s(size_t size); // can`t return NULL.
void *realloc_s(void *ptr, size_t size); // can`t return NULL.

#endif
