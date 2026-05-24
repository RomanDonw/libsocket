#include "libsocket.h"

#include <stdlib.h>

const IPv4Address IPV4ADDR_ANY = IPV4ADDR_INIT(INADDR_ANY);
const IPv4Address IPV4ADDR_LOOPBACK = IPV4ADDR_INIT(INADDR_LOOPBACK);
const IPv4Address IPV4ADDR_BROADCAST = IPV4ADDR_INIT(INADDR_BROADCAST);

const IPv6Address IPV6ADDR_ANY = IN6ADDR_ANY_INIT;
const IPv6Address IPV6ADDR_LOOPBACK = IN6ADDR_LOOPBACK_INIT;

void *(*libsocket_malloc)(size_t) = malloc;
void *(*libsocket_realloc)(void *, size_t) = realloc;
void (*libsocket_free)(void *) = free;