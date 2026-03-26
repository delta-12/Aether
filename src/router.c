#include "router.h"

#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "hash.h"
#include "hashmap.h"
#include "log.h"
#include "random.h"
#include "socket.h"
#include "tick.h"
#include "transport.h"

#ifndef AETHER_SESSION_RETRIES
#define AETHER_SESSION_RETRIES 5U
#endif /* AETHER_SESSION_RETRIES */

#ifndef AETHER_SESSION_LEASE
#define AETHER_SESSION_LEASE (a_Tick_Ms_t)500U
#endif /* AETHER_SESSION_LEASE */

typedef enum
{
    A_ROUTER_SESSION_STATE_CONNECT,
    A_ROUTER_SESSION_STATE_ACCEPT,
    A_ROUTER_SESSION_STATE_OPEN,
    A_ROUTER_SESSION_STATE_CLOSED,
    A_ROUTER_SESSION_STATE_FAILED
} a_Router_SessionState_t;

typedef struct
{
    a_Router_SessionId_t id;
    a_Transport_Header_t header;
    a_Transport_PeerId_t peer_id;
    a_Transport_SequenceNumber_t sequence_number;
    char *key;
    uint8_t *data;
} a_Router_Forwarder_t;

typedef struct
{
    a_Socket_t socket;
    a_Hashmap_t keys;
    a_Router_SessionState_t state;
    size_t retries;
    a_Tick_Ms_t lease;
    a_Tick_Ms_t last_renew_received;
    a_Tick_Ms_t last_renew_sent;
    a_Transport_Message_t message;
} a_Router_Session_t;

static const char *const            a_Router_LogTag         = "ROUTER";
static a_Transport_PeerId_t         a_Router_PeerId         = 0U;
static a_Transport_SequenceNumber_t a_Router_SequenceNumber = 0U;
static a_Hashmap_t                  a_Router_Sessions;
static a_Hashmap_t                  a_Router_SequenceNumbers;
// static a_Hashmap_t                  a_Router_Subscribers; /* TODO replace with trie */

static void a_Router_SerializeMessage(a_Transport_Message_t *const message);
static a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionMessageReceive(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static void a_Router_SessionTaskCallback(void *key, void *value, const void *const arg);
static a_Err_t a_Router_SessionTask(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionConnect(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionOpen(const a_Router_SessionId_t id, a_Router_Session_t *const session);

a_Err_t a_Router_Initialize(const a_Transport_PeerId_t id)
{
    if (A_TRANSPORT_PEER_ID_MAX == id)
    {
        a_Router_PeerId = a_Random_Get32();
    }
    else
    {
        a_Router_PeerId = id;
    }

    A_LOG_DEBUG(a_Router_LogTag, "Peer ID set to %d", a_Router_PeerId);

    a_Err_t error = a_Hashmap_Initialize(&a_Router_Sessions, sizeof(a_Router_SessionId_t), sizeof(a_Router_Session_t));

    if (A_ERR_NONE == error)
    {
        error = a_Hashmap_Initialize(&a_Router_SequenceNumbers, sizeof(a_Transport_PeerId_t), sizeof(a_Transport_SequenceNumber_t));
    }

    if (A_ERR_NONE == error)
    {
        /* TODO initialize trie */
    }

    return error;
}

void a_Router_Deinitialize(void)
{
    a_Hashmap_Deinitialize(&a_Router_Sessions);
    a_Hashmap_Deinitialize(&a_Router_SequenceNumbers);
    /* TODO deinitialize trie */
}

void a_Router_Task(void)
{
    a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionTaskCallback, NULL);
}

a_Err_t a_Router_SessionAdd(const a_Router_SessionId_t id, const a_Socket_t *const socket, uint8_t *const buffer, const size_t size)
{
    a_Err_t error = A_ERR_DUPLICATE;

    if (NULL == socket)
    {
        error = A_ERR_NULL;
    }
    else if (NULL == a_Hashmap_Get(&a_Router_Sessions, &id))
    {
        a_Router_Session_t session;
        error = a_Hashmap_Insert(&a_Router_Sessions, &id, &session);

        if (A_ERR_NONE == error)
        {
            a_Router_Session_t *new_session = (a_Router_Session_t *)a_Hashmap_Get(&a_Router_Sessions, &id);

            /* TODO initialize session hashmap*/
            /* TODO set maximum key string size */
            new_session->socket = *socket;
            new_session->state  = A_ROUTER_SESSION_STATE_CONNECT;

            error = a_Transport_MessageInitialize(&new_session->message, buffer, size);

            A_LOG_DEBUG(a_Router_LogTag, "Session %d added", id);
        }
    }

    return error;
}

a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id)
{
    a_Err_t                   error   = A_ERR_NONE;
    a_Router_Session_t *const session = a_Hashmap_Get(&a_Router_Sessions, &id);

    if (NULL != session)
    {
        a_Transport_MessageReset(&session->message);
        (void)a_Transport_MessageClose(&session->message);
        error = a_Router_SessionMessageSend(id, session);

        // a_Hashmap_ForEach(&a_Router_Subscribers, a_Router_RemoveSubscriberCallback, &id); /* TODO remove any callbacks associated with session if applicable */
        if (A_ERR_NONE == error)
        {
            error = a_Hashmap_Remove(&a_Router_Sessions, &id);
        }
    }

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Session %d failed to delete with error %s", id, a_Err_ToString(error));
    }
    else
    {
        A_LOG_DEBUG(a_Router_LogTag, "Session %d deleted", id);
    }

    return error;
}

static void a_Router_SerializeMessage(a_Transport_Message_t *const message)
{
    (void)a_Transport_SerializeMessage(message, a_Router_PeerId, a_Router_SequenceNumber);
    a_Router_SequenceNumber++;
}

static a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Router_SerializeMessage(&session->message);

    a_Err_t error = a_Socket_Send(&session->socket, a_Transport_GetMessageBuffer(&session->message));

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Session %d sending message with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionMessageReceive(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Transport_MessageReset(&session->message);

    a_Buffer_t *const buffer = a_Transport_GetMessageBuffer(&session->message);
    a_Err_t           error  = a_Socket_Receive(&session->socket, buffer);

    if ((A_ERR_NONE == error) && (a_Buffer_GetReadSize(buffer) > 0U))
    {
        error = a_Transport_DeserializeMessage(&session->message);

        const a_Transport_PeerId_t                peer_id                 = a_Transport_GetMessagePeerId(&session->message);
        const a_Transport_SequenceNumber_t        sequence_number         = a_Transport_GetMessageSequenceNumber(&session->message);
        const a_Transport_SequenceNumber_t *const current_sequence_number = a_Hashmap_Get(&a_Router_SequenceNumbers, &peer_id);

        if (A_ERR_NONE != error)
        {
            /* Error deserializing message */
        }
        else if ((A_TRANSPORT_PEER_ID_MAX == peer_id) || (A_TRANSPORT_SEQUENCE_NUMBER_MAX == sequence_number))
        {
            error = A_ERR_SERIALIZATION;
        }
        else if ((NULL == current_sequence_number) || (sequence_number > *current_sequence_number))
        {
            error = a_Hashmap_Insert(&a_Router_SequenceNumbers, &peer_id, &sequence_number);
        }
    }

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Session %d receiving message with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static void a_Router_SessionTaskCallback(void *key, void *value, const void *const arg)
{
    A_UNUSED(arg);

    (void)a_Router_SessionTask(*(a_Router_SessionId_t *)key, value);
}

static a_Err_t a_Router_SessionTask(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t error = A_ERR_NONE;

    switch (session->state)
    {
    case A_ROUTER_SESSION_STATE_CONNECT:
        error = a_Router_SessionConnect(id, session);
        break;
    case A_ROUTER_SESSION_STATE_ACCEPT:
        error = a_Router_SessionAccept(id, session);
        break;
    case A_ROUTER_SESSION_STATE_OPEN:
        error = a_Router_SessionOpen(id, session);
        break;
    case A_ROUTER_SESSION_STATE_CLOSED:
        A_LOG_DEBUG(a_Router_LogTag, "Session %d closed", id);
        session->state = A_ROUTER_SESSION_STATE_CONNECT;
        break;
    case A_ROUTER_SESSION_STATE_FAILED:
        A_LOG_DEBUG(a_Router_LogTag, "Session %d failed", id);
        session->state = A_ROUTER_SESSION_STATE_CONNECT;
        break;
    default:
        session->state = A_ROUTER_SESSION_STATE_FAILED;
        break;
    }

    return error;
}

static a_Err_t a_Router_SessionConnect(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Transport_MessageReset(&session->message);
    (void)a_Transport_MessageConnect(&session->message, session->lease);

    a_Err_t error = a_Router_SessionMessageSend(id, session);

    if (A_ERR_NONE == error)
    {
        session->state               = A_ROUTER_SESSION_STATE_ACCEPT;
        session->retries             = AETHER_SESSION_RETRIES;
        session->lease               = AETHER_SESSION_LEASE;
        session->last_renew_received = a_Tick_GetTick();

        A_LOG_DEBUG(a_Router_LogTag, "Session %d connecting", id);
    }
    else
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;

        A_LOG_WARNING(a_Router_LogTag, "Session %d failed connecting with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t error = A_ERR_NONE;

    if (session->retries > 0U)
    {
        const a_Tick_Ms_t tick = a_Tick_GetTick();

        if (a_Tick_GetElapsed(session->last_renew_received) > AETHER_SESSION_LEASE)
        {
            session->retries--;
            session->last_renew_received = tick;
        }
        else
        {
            error = a_Router_SessionMessageReceive(id, session);

            if ((A_ERR_NONE != error) || !a_Transport_IsMessageDeserialized(&session->message))
            {
                /* Error receiving message or no message received */
            }
            else if (A_TRANSPORT_HEADER_CONNECT == a_Transport_GetMessageHeader(&session->message))
            {
                /* TODO handle version mismatch and arbitrate MTU, make sure to get fields in correct order */
                a_Tick_Ms_t lease = a_Transport_GetMessageLease(&session->message);
                if (lease < session->lease)
                {
                    session->lease = lease;
                }

                session->last_renew_received = tick;

                (void)a_Transport_MessageAccept(&session->message, session->lease);
                error = a_Router_SessionMessageSend(id, session);
            }
            else if ((A_TRANSPORT_HEADER_ACCEPT == a_Transport_GetMessageHeader(&session->message)) && (a_Transport_GetMessageLease(&session->message) == session->lease))
            {
                /* TODO verify MTU matches */

                session->last_renew_received = tick;
                session->last_renew_sent     = tick;
                session->state               = A_ROUTER_SESSION_STATE_OPEN;

                A_LOG_DEBUG(a_Router_LogTag, "Session %d opened", id);
            }
            else
            {
                error = A_ERR_SEQUENCE;
            }
        }
    }
    else
    {
        session->state = A_ROUTER_SESSION_STATE_CLOSED;

        A_LOG_WARNING(a_Router_LogTag, "Session %d not opened", id);
    }

    if (A_ERR_NONE != error)
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;

        A_LOG_ERROR(a_Router_LogTag, "Session %d failed to open with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionOpen(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t           error = a_Router_SessionMessageReceive(id, session);
    const a_Tick_Ms_t tick  = a_Tick_GetTick();

    if (A_ERR_NONE != error)
    {
        /* Error receiving message */
        A_LOG_WARNING(a_Router_LogTag, "Session %d failed to receive message with error %s", id, a_Err_ToString(error));
    }
    else if (a_Transport_IsMessageDeserialized(&session->message))
    {
        switch (a_Transport_GetMessageHeader(&session->message))
        {
        case A_TRANSPORT_HEADER_CLOSE:
            session->state = A_ROUTER_SESSION_STATE_CLOSED;
            break;
        case A_TRANSPORT_HEADER_RENEW:
            session->last_renew_received = tick;
            break;
        case A_TRANSPORT_HEADER_SUBSCRIBE:
            /* TODO */
            break;
        case A_TRANSPORT_HEADER_PUBLISH:
            /* TODO */
            break;
        case A_TRANSPORT_HEADER_CONNECT:
        case A_TRANSPORT_HEADER_ACCEPT:
        default:
            A_LOG_ERROR(a_Router_LogTag, "Session %d received invalid header", id);
            break;
        }
    }

    if (a_Tick_GetElapsed(session->last_renew_received) > session->lease)
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;
        error          = A_ERR_TIMEOUT;

        A_LOG_ERROR(a_Router_LogTag, "Session %d timed out", id);
    }
    else if (a_Tick_GetElapsed(session->last_renew_sent) > (session->lease / 2U))
    {
        a_Transport_MessageReset(&session->message);
        (void)a_Transport_MessageRenew(&session->message);
        error                    = a_Router_SessionMessageSend(id, session);
        session->last_renew_sent = tick;
    }

    return error;
}
