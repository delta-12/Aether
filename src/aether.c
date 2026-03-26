#include "aether.h"

#include <stddef.h>
#include <stdint.h>

#include "err.h"
#include "log.h"
#include "random.h"
#include "router.h"
#include "socket.h"
#include "transport.h"

a_Err_t a_Initialize(const a_Transport_PeerId_t id)
{
    a_Random_Seed();

    return a_Router_Initialize(id);
}

void a_Deinitialize(void)
{
    a_Router_Deinitialize();
}

void a_SetLogLevel(const a_Log_Level_t level)
{
    a_Log_SetLogLevel(level);
}

a_Err_t a_AddSocket(const a_Socket_t *const socket, const a_Mode_t mode, uint8_t *const message_buffer, const size_t message_buffer_size)
{
    /* TODO set mode */
    A_UNUSED(mode);

    return a_Router_SessionAdd(a_Random_Get32(), socket, message_buffer, message_buffer_size);
}

void a_Task(void)
{
    a_Router_Task();
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
