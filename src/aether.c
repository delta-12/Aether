#include "aether.h"

#include <stddef.h>
#include <stdint.h>

#include "err.h"
#include "hashmap.h"
#include "random.h"
#include "router.h"
#include "session.h"
#include "socket.h"
#include "transport.h"

static a_Hashmap_t a_Sessions;
static uint8_t     a_SessionsData[(sizeof(a_Router_SessionId_t) + sizeof(a_Session_t)) * AETHER_ROUTER_MAX_SESSIONS];

static a_Err_t a_SessionTaskCallback(void *key, void *value, const void *const arg);


a_Err_t a_Initialize(const a_Transport_PeerId_t id)
{
    a_Random_Seed();
    a_Err_t error = a_Hashmap_Initialize(&a_Sessions, a_SessionsData, sizeof(a_SessionsData), sizeof(a_Router_SessionId_t), sizeof(a_Session_t));

    if (A_ERR_NONE == error)
    {
        error = a_Router_Initialize(id);
    }

    return error;
}

a_Err_t a_AddSocket(const a_Socket_t *const socket, const a_Mode_t mode, uint8_t *const message_buffer, const size_t message_buffer_size)
{
    a_Session_t session;
    a_Err_t     error = a_Session_Initialize(&session, socket, message_buffer, message_buffer_size);

    /* TODO set mode */
    A_UNUSED(mode);

    if (A_ERR_NONE == error)
    {
        error = a_Hashmap_Insert(&a_Sessions, &session.id, &session);
    }

    return error;
}

a_Err_t a_Task(void)
{
    /* TODO remove dead sessions */

    return a_Hashmap_ForEach(&a_Sessions, a_SessionTaskCallback, NULL);
}

a_Err_t a_Publish(void)
{
    /* TODO */
    return A_ERR_NONE;
}

a_Err_t a_Subscribe(void)
{
    /* TODO */
    return A_ERR_NONE;
}

a_Err_t a_Query(void)
{
    /* TODO */
    return A_ERR_NONE;
}

static a_Err_t a_SessionTaskCallback(void *key, void *value, const void *const arg)
{
    A_UNUSED(key);
    A_UNUSED(arg);

    return a_Session_Task(value);
}
