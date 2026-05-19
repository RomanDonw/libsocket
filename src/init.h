#ifndef INIT_H
#define INIT_H

#include <stdbool.h>

#include "err.h"

extern bool inited; // !!! readonly !!!

#define ENSURE_INIT(return_value_on_error) { if (!inited) RETURNWITHERROR(SocketError_NotInitialized, return_value_on_error); }

#endif