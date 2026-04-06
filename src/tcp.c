#include "tcp.h"

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"
#include "leb128.h"
#include "socket.h"

#define A_TCP_SIZE_MAX       (uint16_t)UINT16_MAX
#define A_TCP_BYTE_BIT_SHIFT 8U
#define A_TCP_BYTE_BIT_MASK  (uint8_t)0xFFU

a_Err_t a_Tcp_Send(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == socket) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else if ((a_Buffer_GetReadSize(data) > (a_Buffer_GetWriteSize(&socket->send_buffer) - sizeof(A_TCP_SIZE_MAX)))
             || (a_Buffer_GetReadSize(data) > A_TCP_SIZE_MAX))
    {
        error = A_ERR_SIZE;
    }
    else if (a_Buffer_GetReadSize(data) > 0U)
    {
        const size_t size = a_Buffer_GetReadSize(data);

        for (size_t i = sizeof(A_TCP_SIZE_MAX); i > 0U; i--)
        {
            *a_Buffer_GetWrite(&socket->send_buffer) = (size >> ((i - 1U) * A_TCP_BYTE_BIT_SHIFT)) & A_TCP_BYTE_BIT_MASK;
            (void)a_Buffer_SetWrite(&socket->send_buffer, 1U);
        }

        (void)a_Buffer_AppendRight(&socket->send_buffer, data);
        (void)a_Buffer_SetRead(data, size);

        const size_t sent = socket->send(a_Buffer_GetRead(&socket->send_buffer), a_Buffer_GetReadSize(&socket->send_buffer));

        if (SIZE_MAX == sent)
        {
            error = A_ERR_SOCKET;
        }
        else
        {
            (void)a_Buffer_SetRead(&socket->send_buffer, sent);
        }
    }

    return error;
}

a_Err_t a_Tcp_Receive(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == socket) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else
    {
        size_t size = a_Buffer_GetReadSize(&socket->receive_buffer);

        if (size < sizeof(A_TCP_SIZE_MAX))
        {
            const size_t received = socket->receive(a_Buffer_GetWrite(&socket->receive_buffer), (sizeof(A_TCP_SIZE_MAX) - size));

            if (SIZE_MAX == received)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetWrite(&socket->receive_buffer, received);
            }
        }

        size = 0U;

        if (a_Buffer_GetReadSize(&socket->receive_buffer) >= sizeof(A_TCP_SIZE_MAX))
        {
            for (size_t i = 0U; i < sizeof(A_TCP_SIZE_MAX); i++)
            {
                size = (size << (i * A_TCP_BYTE_BIT_SHIFT)) | *(a_Buffer_GetRead(&socket->receive_buffer) + i);
            }

            if (a_Buffer_GetWriteSize(&socket->receive_buffer) < size)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                size -= (a_Buffer_GetReadSize(&socket->receive_buffer) - sizeof(A_TCP_SIZE_MAX));
            }
        }

        if ((A_ERR_NONE == error) && (size > 0U))
        {
            const size_t received = socket->receive(a_Buffer_GetWrite(&socket->receive_buffer), size);

            if (SIZE_MAX == received)
            {
                error = A_ERR_SOCKET;
            }
            else
            {
                (void)a_Buffer_SetWrite(&socket->receive_buffer, received);
            }

            if (received == size)
            {
                (void)a_Buffer_SetRead(&socket->receive_buffer, sizeof(A_TCP_SIZE_MAX));
                error = a_Buffer_Copy(data, &socket->receive_buffer);
            }
        }
    }

    return error;
}
