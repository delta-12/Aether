#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_socket.h"

#include "err.h"
#include "router.h"
#include "socket.h"

#define SERIAL_TEST_BUFFER_SIZE 256U

class Router : public testing::Test
{
protected:
    static a_Err_t Stop(void)
    {
        return mock_socket_->Stop();
    }

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
        a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = Stop, .send = Send, .receive = Receive}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
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

TEST_F(Router, Task)
{
    a_Router_SessionId_t id = 12345678U;
    std::uint8_t connect_message[] = {0x01U, 0x08U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x01U, 0xE8U, 0x07U, 0x00U};
    std::uint8_t accept_message[] = {0x09U, 0x01U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x02U, 0xFAU, 0x01U, 0x00U};
    a_Router_Initialize(A_TRANSPORT_PEER_ID_MAX);

    a_Router_SessionAdd(id, &socket_, message_buffer_, sizeof(message_buffer_), false);
    ASSERT_EQ(A_ERR_DUPLICATE, a_Router_SessionAdd(id, &socket_, message_buffer_, sizeof(message_buffer_), false));
    EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::Return(SIZE_MAX));
    EXPECT_CALL(*mock_socket_, Stop()).Times(1).WillOnce(testing::Return(A_ERR_NONE));
    a_Router_Task(); // Send connect
    a_Router_Task(); // Failed
    a_Router_Task(); // Closed
    ASSERT_EQ(A_ERR_NONE, a_Router_SessionAdd(id, &socket_, message_buffer_, sizeof(message_buffer_), false));
}

TEST_F(Router, SessionDelete)
{
    a_Router_SessionId_t id = 12345678U;
    a_Router_Initialize(A_TRANSPORT_PEER_ID_MAX);

    ASSERT_EQ(A_ERR_NONE, a_Router_SessionDelete(0U));

    a_Router_SessionAdd(id, &socket_, message_buffer_, sizeof(message_buffer_), true);
    EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
    EXPECT_CALL(*mock_socket_, Stop()).Times(1).WillOnce(testing::Return(A_ERR_NONE));
    ASSERT_EQ(A_ERR_NONE, a_Router_SessionDelete(id));

    // TODO
}
