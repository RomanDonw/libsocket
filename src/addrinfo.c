#include "libsocket.h"

#include <string.h>
#include <stddef.h>

#ifndef LIBSOCKET_OS_WINDOWS
    #include <netdb.h>
#endif

#ifdef LIBSOCKET_DEBUG
    #include <stdio.h>
#endif

#include "err.h"

bool socket_getaddrinfo(const char *node, const char *service, const SocketDNSRequest *request, SocketDNSResponse **response)
{
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
        int err = getaddrinfo(node, service, hints, &result);
        switch (err)
        {
            case 0: // success.
                break;

            case EAI_AGAIN:
                RETURNWITHERROR(DNSTemporaryError, false);

            case EAI_NONAME:
                RETURNWITHERROR(DNSHostNotFound, false);

            case EAI_SERVICE:
                RETURNWITHERROR(DNSUnsupportedServiceName, false);

            case EAI_SOCKTYPE:
                RETURNWITHERROR(UnsupportedSocketType, false);

            case EAI_MEMORY:
                RETURNWITHERROR(MemoryAllocationFailed, false);

            case EAI_FAMILY:
                RETURNWITHERROR(UnsupportedAddressFamily, false);

            case EAI_FAIL:
                RETURNWITHERROR(DNSFailure, false);

            case EAI_BADFLAGS:
                RETURNWITHERROR(IncorrectArgumentValue, false);

            #ifdef EAI_OVERFLOW
                case EAI_OVERFLOW:
                    RETURNWITHERROR(InsufficientBufferSize, false);
            #endif

            #ifdef EAI_SYSTEM
                case EAI_SYSTEM:
                    RETURNWITHSYSERR(false);
            #endif

            default:
                #ifdef LIBSOCKET_OS_WINDOWS
                    SocketError serr = translateerror(err);
                    #ifdef LIBSOCKET_DEBUG
                        if (serr == InternalUnknownError) fprintf(stderr, "Got unhandled getaddrinfo error: %i.\n", err);
                    #endif
                    RETURNWITHERROR(serr, false);
                #else
                    #ifdef LIBSOCKET_DEBUG
                        fprintf(stderr, "Got unhandled getaddrinfo error: %i.\n", err);
                    #endif
                    RETURNWITHERROR(InternalUnknownError, false);
                #endif
        }
    }

    #define LOOPALLOCFAILEDHANDLE \
        {\
            freeaddrinfo(result);\
            socket_freeaddrinfo(firstresp); /* safe for NULL (for now xd). */\
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
    return true;
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