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

#define TCP_TEST_SOCKET_BUFFER_SIZE 2048U
#define TCP_TEST_DATA_BUFFER_SIZE 65536U
#define TCP_LENGTH_SIZE sizeof(std::uint16_t)

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

class Tcp : public testing::Test
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
        a_Buffer_Initialize(&buffer_, data_buffer_, sizeof(data_buffer_));
    }

    void TearDown() override
    {
        delete mock_socket_;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    a_Buffer_t buffer_;
    std::uint8_t send_buffer_[TCP_TEST_SOCKET_BUFFER_SIZE];
    std::uint8_t receive_buffer_[TCP_TEST_SOCKET_BUFFER_SIZE];
    std::uint8_t data_buffer_[TCP_TEST_DATA_BUFFER_SIZE];
};

MockSocket *Tcp::mock_socket_ = nullptr;

TEST_F(Tcp, Send)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x00U, 0x03U, 0x04U, 0x05U};
    const std::size_t send_size = sizeof(data) + TCP_LENGTH_SIZE;

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::Return(SIZE_MAX));
        EXPECT_CALL(*mock_socket_, Send(testing::Truly([&data](const std::uint8_t *const sent)
                                                       { return (0 == std::memcmp(sent + 2U, data, sizeof(data))) &&
                                                                (0 == std::memcmp((sent + 2U + send_size), data, sizeof(data))); }),
                                        send_size * 2U))
            .Times(2)
            .WillOnce(testing::Return(send_size))
            .WillOnce(testing::Return(send_size * 2U));
    }

    ASSERT_EQ(A_ERR_NULL, a_Tcp_Send(nullptr, &buffer_));
    ASSERT_EQ(A_ERR_NULL, a_Tcp_Send(&socket_, nullptr));

    a_Buffer_SetWrite(&buffer_, sizeof(data_buffer_));
    ASSERT_EQ(A_ERR_SIZE, a_Tcp_Send(&socket_, &buffer_));

    a_Buffer_Initialize(&buffer_, data, sizeof(data));
    a_Buffer_SetWrite(&buffer_, sizeof(data));
    ASSERT_EQ(A_ERR_SOCKET, a_Tcp_Send(&socket_, &buffer_));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer_));

    a_Buffer_SetWrite(&buffer_, sizeof(data));
    ASSERT_EQ(A_ERR_NONE, a_Tcp_Send(&socket_, &buffer_));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer_));
    ASSERT_EQ(send_size, a_Buffer_GetReadSize(&socket_.send_buffer));

    a_Buffer_SetWrite(&buffer_, sizeof(data));
    ASSERT_EQ(A_ERR_NONE, a_Tcp_Send(&socket_, &buffer_));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer_));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&socket_.send_buffer));
}

TEST_F(Tcp, Receive)
{
    ASSERT_EQ(A_ERR_NULL, a_Tcp_Receive(nullptr, &buffer_));
    ASSERT_EQ(A_ERR_NULL, a_Tcp_Receive(&socket_, nullptr));

    // TODO
}
