#include <cstddef>
#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "buffer.h"
#include "err.h"
#include "socket.h"

#define SOCKET_TEST_BUFFER_SIZE 2048U

class AbstractSocket
{
public:
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(uint8_t *const data, const size_t size) const = 0;
};

class MockSocket : public AbstractSocket
{
public:
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (uint8_t *const data, const size_t size), (const, override));
};

class Socket : public testing::Test
{
protected:
    static std::size_t Send(const std::uint8_t *const data, const std::size_t size)
    {
        return mock_socket_->Send(data, size);
    }

    static std::size_t Receive(uint8_t *const data, const size_t size)
    {
        return mock_socket_->Receive(data, size);
    }

    void SetUp() override
    {
        mock_socket_ = new MockSocket;
    }

    void TearDown() override
    {
        delete mock_socket_;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[SOCKET_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[SOCKET_TEST_BUFFER_SIZE];
};

MockSocket *Socket::mock_socket_ = nullptr;

TEST_F(Socket, Initialize)
{
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(nullptr, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, nullptr, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, nullptr, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), nullptr, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, nullptr, sizeof(receive_buffer_)));

    ASSERT_EQ(A_ERR_SIZE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, 2U, Receive, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_SIZE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, 2U));

    ASSERT_EQ(A_ERR_NONE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_)));
}

TEST_F(Socket, Send)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x00U, 0x03U, 0x04U, 0x05U};
    a_Buffer_t buffer;

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Buffer_SetWrite(&buffer, sizeof(data));
    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_));

    ASSERT_EQ(A_ERR_NULL, a_Socket_Send(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Send(&socket_, nullptr));
}

TEST_F(Socket, Receive)
{
    std::uint8_t data[6U];
    a_Buffer_t buffer;

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_));

    ASSERT_EQ(A_ERR_NULL, a_Socket_Receive(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Receive(&socket_, nullptr));
}
