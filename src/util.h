#ifndef UTIL_H
#define UTIL_H

#include "libsocket.h"

int fillsockaddrstruct(struct sockaddr *out_sockaddr, SocketAddressFamily af, const char *addr, unsigned short port);

#endif