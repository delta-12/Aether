#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "aether.h"

#define SERIAL_TEST_BUFFER_SIZE 256U

class Socket
{
public:
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(std::uint8_t *const data, const std::size_t size) const = 0;
};

class Subscriber
{
public:
    virtual void Callback(const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg) const = 0;
};

class MockSocket : public Socket
{
public:
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (std::uint8_t *const data, const std::size_t size), (const, override));
};

class MockSubscriber : public Subscriber
{
public:
    MOCK_METHOD(void, Callback, (const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg), (const, override));
};

class Aether : public testing::Test
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

    static void Callback(const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg)
    {
        return mock_subscriber_->Callback(key, data, size, arg);
    }

    void SetUp() override
    {
        mock_socket_ = new MockSocket;
        mock_subscriber_ = new MockSubscriber;
        a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer_, sizeof(send_buffer_), Receive, receive_buffer_, sizeof(receive_buffer_));
    }

    void TearDown() override
    {
        a_Deinitialize();
        delete mock_socket_;
        delete mock_subscriber_;
    }

    static MockSocket *mock_socket_;
    static MockSubscriber *mock_subscriber_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t message_buffer_[AETHER_TRANSPORT_MTU];
};

MockSocket *Aether::mock_socket_ = nullptr;
MockSubscriber *Aether::mock_subscriber_ = nullptr;

TEST_F(Aether, Initialize)
{
    ASSERT_EQ(A_ERR_NONE, a_Initialize(A_TRANSPORT_PEER_ID_MAX));
}

TEST_F(Aether, AddSocket)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);

    ASSERT_EQ(A_ERR_NULL, a_AddSocket(nullptr, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_AddSocket(&socket_, A_MODE_CONNECT, nullptr, sizeof(message_buffer_)));
    ASSERT_EQ(A_ERR_NONE, a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_)));
}

TEST_F(Aether, Task)
{
    /* TODO ensure random peer ID, current peer id = 12345678U */
    std::uint8_t connect_message[] = {0x01U, 0x05U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x03U, 0xE8U, 0x07U, 0x00U};
    std::uint8_t accept_message[] = {0x09U, 0x01U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x01U, 0xF4U, 0x03U, 0x00U};
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_));

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0; i < sizeof(connect_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(connect_message[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0; i < sizeof(accept_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(accept_message[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U)).Times(1).WillOnce(testing::Return(0U));
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
    }

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/foo", Callback, nullptr));

    a_Task();
    a_Task();
    a_Task();

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/bar", Callback, nullptr));
}

TEST_F(Aether, Publish)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x03U, 0x04U};
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_));

    ASSERT_EQ(A_ERR_NULL, a_Publish(nullptr, data, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Publish("/foo", nullptr, sizeof(data)));

    ASSERT_EQ(A_ERR_SIZE, a_Publish("/foo", data, 0U));

    ASSERT_EQ(A_ERR_NONE, a_Publish("/foo", data, sizeof(data)));
}

TEST_F(Aether, Subscribe)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, A_MODE_CONNECT, message_buffer_, sizeof(message_buffer_));

    ASSERT_EQ(A_ERR_NULL, a_Subscribe(nullptr, Callback, nullptr));
    ASSERT_EQ(A_ERR_NULL, a_Subscribe("/foo", nullptr, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/foo", Callback, nullptr));
}
