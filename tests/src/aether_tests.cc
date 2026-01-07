#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "aether.h"

#define SERIAL_TEST_BUFFER_SIZE 256U

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

class Aether : public testing::Test
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
    std::uint8_t send_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t message_buffer_[AETHER_TRANSPORT_MTU];
};

MockSocket *Aether::mock_socket_ = nullptr;

TEST_F(Aether, Initialize)
{
    ASSERT_EQ(A_ERR_NONE, a_Initialize(A_TRANSPORT_PEER_ID_MAX));
}

TEST_F(Aether, AddSocket)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);

    ASSERT_EQ(A_ERR_NULL, a_AddSocket(NULL, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_AddSocket(&socket_, A_MODE_CONNECT, NULL, sizeof(message_buffer_)));
    ASSERT_EQ(A_ERR_NONE, a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_)));
}

TEST_F(Aether, Task)
{
    std::uint8_t accept_message[] = {0x09U, 0x01U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x01U, 0xE8U, 0x07U, 0x00U}; /* TODO ensure random peer ID */
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_));

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0; i < sizeof(accept_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(accept_message[i]), testing::Return(1U)));
        }
    }

    ASSERT_EQ(A_ERR_NONE, a_Task());
    ASSERT_EQ(A_ERR_NONE, a_Task());
}
