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
            return Success;

        case EAI_AGAIN:
            return DNSTemporaryError;

        case EAI_NONAME:
            return DNSHostNotFound;

        case EAI_SERVICE:
            return DNSUnsupportedServiceName;

        case EAI_SOCKTYPE:
            return UnsupportedSocketType;

        case EAI_MEMORY:
            return MemoryAllocationFailed;

        case EAI_FAMILY:
            return UnsupportedAddressFamily;

        case EAI_FAIL:
            return DNSFailure;

        case EAI_BADFLAGS:
            return IncorrectArgumentValue;

        #ifdef EAI_OVERFLOW
            case EAI_OVERFLOW:
                return InsufficientBufferSize;
        #endif

        #ifdef EAI_SYSTEM
            case EAI_SYSTEM:
        #endif
        default:
            return translateerror(err);
    }
}

static void __socket_freeaddrinfo(SocketDNSResponse *response)
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

bool socket_getaddrinfo(const char *nodename, const char *servicename, const SocketDNSRequest *request, SocketDNSResponse **response)
{
    ENSURE_INIT(false);
    
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
        if (err != Success) RETURNWITHERROR(err, false);
    }

    #define LOOPALLOCFAILEDHANDLE \
        {\
            freeaddrinfo(result);\
            if (firstresp) __socket_freeaddrinfo(firstresp);\
            RETURNWITHERROR(MemoryAllocationFailed, false);\
        }

    SocketDNSResponse *firstresp = NULL;
    SocketDNSResponse *currresp = NULL;
    for (struct addrinfo *currai = result; currai; currai = currai->ai_next)
    {
        SocketDNSResponse *resp = libsocket_malloc(sizeof(SocketDNSResponse));
        if (!resp) LOOPALLOCFAILEDHANDLE;

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
            if (!resp->canonname) { libsocket_free(resp); LOOPALLOCFAILEDHANDLE; }
            memcpy(resp->canonname, currai->ai_canonname, size);
        }
        else resp->canonname = NULL;

        // =============================================================================

        if (resp->sockaddrlen && currai->ai_addr)
        {
            resp->sockaddr = libsocket_malloc(resp->sockaddrlen);
            if (!resp->sockaddr)
            {
                if (resp->canonname) libsocket_free(resp->canonname);
                libsocket_free(resp);
                LOOPALLOCFAILEDHANDLE;
            }
            memcpy(resp->sockaddr, currai->ai_addr, resp->sockaddrlen);
        }
        else resp->sockaddr = NULL;

        // =============================================================================

        if (currresp) currresp->next = resp;
        currresp = resp;
        if (!firstresp) firstresp = currresp;
    }

    #undef LOOPALLOCFAILEDHANDLE

    freeaddrinfo(result);
    *response = firstresp;
    RETURNWITHSUCCESS(true);
}

bool socket_freeaddrinfo(SocketDNSResponse *response)
{
    ENSURE_INIT(false);
    if (!response) RETURNWITHERROR(Fault, false);
    __socket_freeaddrinfo(response);
    RETURNWITHSUCCESS(true);
}

bool socket_getnameinfo(const SocketAddressInterface *sockaddr, socklen_t sockaddrlen, char *nodename, uint32_t nodenamesize, char *servicename, uint32_t servicenamesize, int flags)
{
    ENSURE_INIT(false);
    SocketError err = translateeaierror(getnameinfo(sockaddr, sockaddrlen, nodename, nodenamesize, servicename, servicenamesize, flags));
    if (err) RETURNWITHERROR(err, false);
    RETURNWITHSUCCESS(true);
}