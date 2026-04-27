#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#include "libsocket.h"

void waitms(uint32_t milliseconds);
void handleerror(const char *funcname);

#endif