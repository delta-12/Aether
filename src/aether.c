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

static const char *const a_LogTag = "AETHER";

a_Err_t a_Initialize(const a_Transport_PeerId_t id)
{
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
}

void a_SetLogLevel(const a_Log_Level_t level)
{
    a_Log_SetLogLevel(level);
}

void a_EnableRouting(const bool enable)
{
    a_Routing_EnableRouting(enable);
}

a_Err_t a_AddSocket(const a_Socket_t *const socket, uint8_t *const message_buffer, const size_t message_buffer_size, const bool retain)
{
    return a_Router_SessionAdd(a_Random_Get32(), socket, message_buffer, message_buffer_size, retain);
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

a_Err_t a_Query(void)
{
    /* TODO */
    return A_ERR_MAX;
}
