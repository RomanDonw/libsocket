/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "libsocket.h"

#include <string.h>
#include <stddef.h>

#ifdef LIBSOCKET_DEBUG
    #include <stdio.h>
#endif

#include "init.h"
#include "err.h"

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
            return SocketError_IncorrectArgumentValue;

        #ifdef EAI_OVERFLOW
            case EAI_OVERFLOW:
                return SocketError_InsufficientBufferSize;
        #endif

        #ifdef EAI_SYSTEM
            case EAI_SYSTEM:
        #endif
        default:
            return translateerror(err);
    }
}

SocketError socket_getaddrinfo(const char *nodename, const char *servicename, const SocketDNSRequest *request, SocketDNSResponse **response)
{
    ENSURE_INIT;
    
    struct addrinfo *hints = NULL;
    struct addrinfo _hints = {0};
    if (request)
    {
        hints = &_hints;

        hints->ai_flags = request->flags;
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
        SocketDNSResponse *resp = libsocket_malloc(sizeof(SocketDNSResponse));
        if (!resp) goto allocfail_resp;

        // =============================================================================

        resp->next = NULL;
        resp->flags = currai->ai_flags;
        resp->af = currai->ai_family;
        resp->type = currai->ai_socktype;
        resp->protocol = currai->ai_protocol;
        resp->sockaddrlen = currai->ai_addrlen;

        // =============================================================================

        if (currai->ai_canonname)
        {
            size_t size = strlen(currai->ai_canonname) + 1;
            resp->canonname = libsocket_malloc(size);
            if (!resp->canonname) goto allocfail_cannonname;
            memcpy(resp->canonname, currai->ai_canonname, size);
        }
        else resp->canonname = NULL;

        // =============================================================================

        if (resp->sockaddrlen && currai->ai_addr)
        {
            resp->sockaddr = libsocket_malloc(resp->sockaddrlen);
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
            if (resp->canonname) libsocket_free(resp->canonname);
        allocfail_cannonname:
            libsocket_free(resp);
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

        libsocket_free(_resp->canonname);
        libsocket_free(_resp->sockaddr);

        libsocket_free(_resp); 
    }
}

SocketError socket_getnameinfo(const SocketAddressInterface *sockaddr, socklen_t sockaddrlen, char *nodename, uint32_t nodenamesize, char *servicename, uint32_t servicenamesize, int flags)
{
    ENSURE_INIT;
    return translateeaierror(getnameinfo(sockaddr, sockaddrlen, nodename, nodenamesize, servicename, servicenamesize, flags));
}