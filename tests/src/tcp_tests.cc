#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "buffer.h"
#include "err.h"
#include "tcp.h"
#include "socket.h"

#define TCP_TEST_BUFFER_SIZE 256U // TODO

class Socket
{
public:
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(uint8_t *const data, const size_t size) const = 0;
};

class MockSocket : public Socket
{
public:
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (uint8_t *const data, const size_t size), (const, override));
};

class Tcp : public testing::Test
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
        a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_));
    }

    void TearDown() override
    {
        delete mock_socket_;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[TCP_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[TCP_TEST_BUFFER_SIZE];
};

MockSocket *Tcp::mock_socket_ = nullptr;

TEST_F(Tcp, Send)
{
    a_Buffer_t buffer;

    ASSERT_EQ(A_ERR_NULL, a_Tcp_Send(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Tcp_Send(&socket_, nullptr));

    // TODO
}

TEST_F(Tcp, Receive)
{
    a_Buffer_t buffer;

    ASSERT_EQ(A_ERR_NULL, a_Tcp_Receive(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Tcp_Receive(&socket_, nullptr));

    // TODO
}
