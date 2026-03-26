#ifndef AETHER_H
#define AETHER_H

#include <stddef.h>
#include <stdint.h>

#include "err.h"
#include "socket.h"
#include "transport.h"

typedef enum
{
    A_MODE_CONNECT,
    A_MODE_LISTEN
} a_Mode_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Initialize(const a_Transport_PeerId_t id);
void a_Deinitialize(void);
a_Err_t a_AddSocket(const a_Socket_t *const socket, const a_Mode_t mode, uint8_t *const message_buffer, const size_t message_buffer_size);
void a_Task(void);
a_Err_t a_Publish(const char *const key, const uint8_t *const data, const size_t size);
a_Err_t a_Subscribe(const char *const key, void (*callback)(const char *const key, const uint8_t *const data, const size_t size, void *arg), void *arg);
a_Err_t a_Query(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_H */
