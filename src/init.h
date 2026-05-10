#ifndef INIT_H
#define INIT_H

#include <stdbool.h>

#include "err.h"

extern bool inited;

#define ENSURE_INIT(return_value_on_error) { if (!inited) RETURNWITHERROR(NotInitialized, return_value_on_error); }

#endif