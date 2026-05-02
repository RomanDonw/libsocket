#ifndef BASE_H
#define BASE_H

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>

#include "libsocket.h"

void waitms(uint32_t milliseconds);
void handleerror(const char *funcname);

#endif
