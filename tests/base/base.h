/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef BASE_H
#define BASE_H

#define _POSIX_C_SOURCE 199309L

#include "libsocket.h"

#include <stdint.h>
#include <stdbool.h>

extern const char *testname;
extern void test(void);

// frees reason before calling abort function.
void testabort_f(char *reason); // reason can be NULL.

// don't frees reason before calling abort function.
void testabort_c(const char *reason); // reason can be NULL.

void waitms(uint32_t milliseconds);
void handlesockerror(NError err, const char *funcname);

void *malloc_s(size_t size); // can`t return NULL.
void *realloc_s(void *ptr, size_t size); // can`t return NULL.

#endif
