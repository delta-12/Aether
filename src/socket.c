#include "socket.h"

#include <stddef.h>
#include <stdint.h>

#include "buffer.h"
#include "err.h"
#include "serial.h"
#include "tcp.h"

#define A_SOCKET_BUFFER_SIZE_MIN (size_t)3U

a_Err_t a_Socket_Initialize(a_Socket_t *const socket,
                            const a_Socket_Type_t type,
                            const a_Socket_Functions_t functions,
                            uint8_t *const send_buffer,
                            const size_t send_buffer_size,
                            uint8_t *const receive_buffer,
                            const size_t receive_buffer_size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == socket) || (NULL == functions.send) || (NULL == functions.receive))
    {
        error = A_ERR_NULL;
    }
    else if ((send_buffer_size < A_SOCKET_BUFFER_SIZE_MIN) || (receive_buffer_size < A_SOCKET_BUFFER_SIZE_MIN))
    {
        error = A_ERR_SIZE;
    }
    else
    {
        socket->type      = type;
        socket->functions = functions;

        error = a_Buffer_Initialize(&socket->send_buffer, send_buffer, send_buffer_size);

        if (A_ERR_NONE == error)
        {
            error = a_Buffer_Initialize(&socket->receive_buffer, receive_buffer, receive_buffer_size);
        }
    }

    return error;
}

a_Err_t a_Socket_Start(a_Socket_t *const socket)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == socket)
    {
        error = A_ERR_NULL;
    }
    else if (NULL != socket->functions.start)
    {
        error = socket->functions.start();
    }

    return error;
}

a_Err_t a_Socket_Stop(a_Socket_t *const socket)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == socket)
    {
        error = A_ERR_NULL;
    }
    else if (NULL != socket->functions.stop)
    {
        error = socket->functions.stop();
    }

    return error;
}

a_Err_t a_Socket_Send(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != socket) && (NULL != data))
    {
        switch (socket->type)
        {
        case A_SOCKET_TYPE_TCP:
            error = a_Tcp_Send(socket, data);
            break;
        case A_SOCKET_TYPE_SERIAL:
            error = a_Serial_Send(socket, data);
            break;
        default:
            error = A_ERR_SOCKET;
            break;
        }
    }

    return error;
}

a_Err_t a_Socket_Receive(a_Socket_t *const socket, a_Buffer_t *const data)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != socket) && (NULL != data))
    {
        switch (socket->type)
        {
        case A_SOCKET_TYPE_TCP:
            error = a_Tcp_Receive(socket, data);
            break;
        case A_SOCKET_TYPE_SERIAL:
            error = a_Serial_Receive(socket, data);
            break;
        default:
            error = A_ERR_SOCKET;
            break;
        }
    }

    return error;
}
