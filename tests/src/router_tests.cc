#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "err.h"
#include "router.h"
#include "socket.h"

#define SERIAL_TEST_BUFFER_SIZE 256U

class Socket
{
public:
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(std::uint8_t *const data, const std::size_t size) const = 0;
};

class MockSocket : public Socket
{
public:
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (std::uint8_t *const data, const std::size_t size), (const, override));
};

class Router : public testing::Test
{
protected:
    static std::size_t Send(const std::uint8_t *const data, const std::size_t size)
    {
        return mock_socket_->Send(data, size);
    }

    static std::size_t Receive(std::uint8_t *const data, const std::size_t size)
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
        a_Router_Deinitialize();
        delete mock_socket_;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t message_buffer_[AETHER_TRANSPORT_MTU];
};

MockSocket *Router::mock_socket_ = nullptr;

TEST_F(Router, SessionDelete)
{
    a_Router_SessionId_t id = 12345678U;
    a_Router_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_Router_SessionAdd(id, &socket_, message_buffer_, sizeof(message_buffer_));

    EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
    ASSERT_EQ(A_ERR_NONE, a_Router_SessionDelete(id));

    /* TODO */
}
