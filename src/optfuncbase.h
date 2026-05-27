/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef OPTFUNCBASE_H
#define OPTFUNCBASE_H

#include "libsocket.h"

#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "init.h"
#include "socket.h"

#ifndef LIBSOCKET_OS_WINDOWS
    #include <sys/time.h>
#endif

#ifdef __clang__
    #define SWMISSDEFAULTFIX default: break;
#else
    #define SWMISSDEFAULTFIX
#endif

#endif
