/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE 

#include "libsocket.h"

#include <string.h>
#include <stddef.h>

#include "init.h"
#include "err.h"
#include "util.h"

static SocketError translateeaierror(int err);

SocketError socket_getaddrinfo(const char *nodename, const char *servicename, const SocketDNSRequest *request, SocketDNSResponse **response)
{
    ENSURE_INIT;
    
    struct addrinfo *hints = NULL;
    struct addrinfo _hints = {0};
    if (request)
    {
        hints = &_hints;

        hints->ai_flags = 0;
        if (request->flags & SOCKET_AI_FLAG_PASSIVE) hints->ai_flags |= AI_PASSIVE;
        if (request->flags & SOCKET_AI_FLAG_CANONNAME) hints->ai_flags |= AI_CANONNAME;
        if (request->flags & SOCKET_AI_FLAG_NUMERICHOST) hints->ai_flags |= AI_NUMERICHOST;
        if (request->flags & SOCKET_AI_FLAG_NUMERICSERV) hints->ai_flags |= AI_NUMERICSERV;
        if (request->flags & SOCKET_AI_FLAG_ADDRCONFIG) hints->ai_flags |= AI_ADDRCONFIG;
        if (request->flags & SOCKET_AI_FLAG_IPV4MAPPED) hints->ai_flags |= AI_V4MAPPED;
        if (request->flags & SOCKET_AI_FLAG_BOTHIPVERS) hints->ai_flags |= AI_ALL;

        hints->ai_family = request->af;
        hints->ai_socktype = request->type;
        hints->ai_protocol = request->protocol;
    }

    struct addrinfo *result;
    {
        SocketError err = translateeaierror(getaddrinfo(nodename, servicename, hints, &result));
        if (err != SocketError_Success) return err;
    }

    SocketDNSResponse *firstresp = NULL;
    SocketDNSResponse *currresp = NULL;
    for (struct addrinfo *currai = result; currai; currai = currai->ai_next)
    {
        SocketDNSResponse *resp = allocs.malloc(sizeof(SocketDNSResponse));
        if (!resp) goto allocfail_resp;

        // =============================================================================

        resp->next = NULL;
        resp->af = currai->ai_family;
        resp->type = currai->ai_socktype;
        resp->protocol = currai->ai_protocol;
        resp->sockaddrlen = currai->ai_addrlen;

        resp->flags = 0;
        if (currai->ai_flags & AI_PASSIVE) resp->flags |= SOCKET_AI_FLAG_PASSIVE;
        if (currai->ai_flags & AI_CANONNAME) resp->flags |= SOCKET_AI_FLAG_CANONNAME;
        if (currai->ai_flags & AI_NUMERICHOST) resp->flags |= SOCKET_AI_FLAG_NUMERICHOST;
        if (currai->ai_flags & AI_NUMERICSERV) resp->flags |= SOCKET_AI_FLAG_NUMERICSERV;
        if (currai->ai_flags & AI_ADDRCONFIG) resp->flags |= SOCKET_AI_FLAG_ADDRCONFIG;
        if (currai->ai_flags & AI_V4MAPPED) resp->flags |= SOCKET_AI_FLAG_IPV4MAPPED;
        if (currai->ai_flags & AI_ALL) resp->flags |= SOCKET_AI_FLAG_BOTHIPVERS;

        // =============================================================================

        if (currai->ai_canonname)
        {
            size_t size = strlen(currai->ai_canonname) + 1;
            resp->canonname = allocs.malloc(size);
            if (!resp->canonname) goto allocfail_cannonname;
            memcpy(resp->canonname, currai->ai_canonname, size);
        }
        else resp->canonname = NULL;

        // =============================================================================

        if (resp->sockaddrlen && currai->ai_addr)
        {
            resp->sockaddr = allocs.malloc(resp->sockaddrlen);
            if (!resp->sockaddr) goto allocfail_sockaddr;
            memcpy(resp->sockaddr, currai->ai_addr, resp->sockaddrlen);
        }
        else resp->sockaddr = NULL;

        // =============================================================================

        if (currresp) currresp->next = resp;
        currresp = resp;
        if (!firstresp) firstresp = currresp;

        // =============================================================================
        continue;
        // =============================================================================

        allocfail_sockaddr:
            if (resp->canonname) allocs.free(resp->canonname);
        allocfail_cannonname:
            allocs.free(resp);
        allocfail_resp:
            freeaddrinfo(result);
            socket_freeaddrinfo(firstresp);
            return SocketError_MemoryAllocationFailed;
    }

    freeaddrinfo(result);
    *response = firstresp;
    return SocketError_Success;
}

void socket_freeaddrinfo(SocketDNSResponse *response)
{
    for (SocketDNSResponse *resp = response; resp;)
    {
        SocketDNSResponse *_resp = resp;
        resp = resp->next;

        allocs.free(_resp->canonname);
        allocs.free(_resp->sockaddr);

        allocs.free(_resp); 
    }
}

SocketError socket_getnameinfo(const SocketAddressInterface *sockaddr, socklen_t sockaddrlen, char *hostname, size_t *hostnamesize, char *servicename, size_t *servicenamesize, SocketGetNameInfoFlags flags)
{
    ENSURE_INIT;
    SocketError err;

    if (!hostnamesize && !servicenamesize) return SocketError_IncorrectArgumentValue;

    int intrflags = 0;
    if (flags & SOCKET_NI_FLAG_NOFQDN) intrflags |= NI_NOFQDN;
    if (flags & SOCKET_NI_FLAG_NUMERICHOST) intrflags |= NI_NUMERICHOST;
    if (flags & SOCKET_NI_FLAG_NUMERICSERV) intrflags |= NI_NUMERICSERV;
    if (flags & SOCKET_NI_FLAG_DGRAM) intrflags |= NI_DGRAM;
    if (flags & SOCKET_NI_FLAG_NAMEREQD) intrflags |= NI_NAMEREQD;

    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];
    if ((err = translateeaierror(getnameinfo(sockaddr, sockaddrlen, host, sizeof(host), serv, sizeof(serv), intrflags))) != SocketError_Success) return err;
    size_t hostlen = strlen(host);
    size_t servlen = strlen(serv);

    if (hostnamesize)
    {
        if (*hostnamesize && hostname)
        {
            if (*hostnamesize > 1) memcpy(hostname, host, (*hostnamesize - 1 > hostlen) ? hostlen : (*hostnamesize - 1));
            hostname[*hostnamesize - 1] = '\0';
        }
        *hostnamesize = hostlen + 1;
    }

    if (servicenamesize)
    {
        if (*servicenamesize && servicename)
        {
            if (*servicenamesize > 1) memcpy(servicename, serv, (*servicenamesize - 1 > servlen) ? servlen : (*servicenamesize - 1));
            servicename[*servicenamesize - 1] = '\0';
        }
        *servicenamesize = servlen + 1;
    }
    
    return SocketError_Success;
}

static SocketError translateeaierror(int err)
{
    switch (err)
    {
        case 0:
            return SocketError_Success;

        case EAI_AGAIN:
            return SocketError_DNSTemporaryError;

        case EAI_NONAME:
            return SocketError_DNSHostNotFound;

        case EAI_SERVICE:
            return SocketError_DNSUnsupportedServiceName;

        case EAI_SOCKTYPE:
            return SocketError_UnsupportedSocketType;

        case EAI_MEMORY:
            return SocketError_MemoryAllocationFailed;

        case EAI_FAMILY:
            return SocketError_UnsupportedAddressFamily;

        case EAI_FAIL:
            return SocketError_DNSFailure;

        case EAI_BADFLAGS:
            return SocketError_BadFlags;

        #ifdef EAI_OVERFLOW
            case EAI_OVERFLOW:
                return SocketError_InsufficientBufferSize;
        #endif

        #ifdef EAI_SYSTEM
            case EAI_SYSTEM:
                return GETLASTTRANSLATEDSYSERR();
        #endif

        default:
            LOGDBGERR("got unhandled IETF (getaddrinfo/getnameinfo) error: %i", err);
            return SocketError_InternalUnknownError;
    }
}
