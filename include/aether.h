#ifndef AETHER_H
#define AETHER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "err.h"
#include "log.h"
#include "router.h"
#include "socket.h"
#include "transport.h"

typedef a_Router_SessionId_t a_Session_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Initialize(const a_Transport_PeerId_t id);
void a_Deinitialize(void);
void a_SetLogLevel(const a_Log_Level_t level);
void a_EnableRouting(const bool enable);
a_Err_t a_AddSession(a_Session_t *const session, const a_Socket_t *const socket, uint8_t *const message_buffer, const size_t message_buffer_size, const bool retain);
a_Err_t a_DeleteSession(a_Session_t *const session);
void a_Task(void);
a_Err_t a_Declare(const char *const key);
a_Err_t a_Publish(const char *const key, const uint8_t *const data, const size_t size);
a_Err_t a_Subscribe(const char *const key, void (*callback)(const char *const key, const uint8_t *const data, const size_t size, void *arg), void *arg);
a_Err_t a_Unsubscribe(const char *const key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_H */
