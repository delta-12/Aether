#include "aether.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "err.h"
#include "log.h"
#include "random.h"
#include "router.h"
#include "socket.h"
#include "transport.h"
#include "version.h"

static const char *const a_LogTag = "AETHER";

a_Err_t a_Initialize(const a_Transport_PeerId_t id)
{
    A_LOG_DEBUG(a_LogTag,
                "Aether Info:\n"
                "    Branch: %s\n"
                "    Commit: %s (%s)\n"
                "    Tag: %s",
                AETHER_GIT_BRANCH,
                AETHER_GIT_COMMIT_HASH,
                AETHER_GIT_DIRTY,
                AETHER_GIT_TAG);

    a_Random_Seed();

    a_Err_t error = a_Router_Initialize(id);

    if (A_ERR_NONE == error)
    {
        A_LOG_INFO(a_LogTag, "Initialized");
    }
    else
    {
        A_LOG_ERROR(a_LogTag, "Failed to initialized with error %s", a_Err_ToString(error));
    }

    return error;
}

void a_Deinitialize(void)
{
    a_Router_Deinitialize();

    A_LOG_INFO(a_LogTag, "Deinitialized");
}

void a_SetLogLevel(const a_Log_Level_t level)
{
    a_Log_SetLogLevel(level);
}

void a_EnableRouting(const bool enable)
{
    a_Router_RoutingEnable(enable);
}

void a_RegisterEventHandler(void (*event_handler)(const a_Event_t event, const a_Session_t *const session, const a_Err_t *const error, void *arg), void *arg)
{
    a_Router_EventHandlerRegister(event_handler, arg);
}

a_Err_t a_InitializeSocket(a_Socket_t *const socket,
                           const a_SocketType_t type,
                           const a_SocketFunctions_t functions,
                           uint8_t *const send_buffer,
                           const size_t send_buffer_size,
                           uint8_t *const receive_buffer,
                           const size_t receive_buffer_size)
{
    return a_Socket_Initialize(socket, type, functions, send_buffer, send_buffer_size, receive_buffer, receive_buffer_size);
}

a_Err_t a_AddSession(a_Session_t *const session, const a_Socket_t *const socket, uint8_t *const message_buffer, const size_t message_buffer_size, const bool retain)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != session)
    {
        *session = a_Random_Get32();
        error    = a_Router_SessionAdd(*session, socket, message_buffer, message_buffer_size, retain);
    }

    return error;
}

a_Err_t a_DeleteSession(a_Session_t *const session)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != session)
    {
        error = a_Router_SessionDelete(*session);
    }

    return error;
}

void a_Task(void)
{
    a_Router_Task();
}

a_Err_t a_Declare(const char *const key)
{
    return a_Router_Declare(key);
}

a_Err_t a_Publish(const char *const key, const uint8_t *const data, const size_t size)
{
    return a_Router_Publish(key, data, size);
}

a_Err_t a_Subscribe(const char *const key, void (*callback)(const char *const key, const uint8_t *const data, const size_t size, void *arg), void *arg)
{
    return a_Router_Subscribe(key, callback, arg);
}

a_Err_t a_Unsubscribe(const char *const key)
{
    return a_Router_Unsubscribe(key);
}
