#include <cstddef>
#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_socket.h"

#include "buffer.h"
#include "err.h"
#include "socket.h"

#define SOCKET_TEST_BUFFER_SIZE 2048U

class Socket : public testing::Test
{
protected:
    static a_Err_t Start(void *arg)
    {
        return mock_socket_->Start(arg);
    }

    static a_Err_t Stop(void *arg)
    {
        return mock_socket_->Stop(arg);
    }

    static std::size_t Send(const std::uint8_t *const data, const std::size_t size, void *arg)
    {
        return mock_socket_->Send(data, size, arg);
    }

    static std::size_t Receive(uint8_t *const data, const size_t size, void *arg)
    {
        return mock_socket_->Receive(data, size, arg);
    }

    void SetUp() override
    {
        mock_socket_ = new MockSocket;
        arg_ = nullptr;
    }

    void TearDown() override
    {
        delete mock_socket_;
        arg_ = nullptr;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[SOCKET_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[SOCKET_TEST_BUFFER_SIZE];
    void *arg_;
};

MockSocket *Socket::mock_socket_ = nullptr;

TEST_F(Socket, Initialize)
{
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(nullptr, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = nullptr, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = nullptr, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, nullptr, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), nullptr, sizeof(receive_buffer_)));

    ASSERT_EQ(A_ERR_SIZE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, 2U, receive_buffer_, sizeof(receive_buffer_)));
    ASSERT_EQ(A_ERR_SIZE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, 2U));

    ASSERT_EQ(A_ERR_NONE, a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_)));
}

TEST_F(Socket, Start)
{
    EXPECT_CALL(*mock_socket_, Start(arg_)).Times(1).WillOnce(testing::Return(A_ERR_MAX));

    ASSERT_EQ(A_ERR_NULL, a_Socket_Start(nullptr));

    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
    ASSERT_EQ(A_ERR_NONE, a_Socket_Start(&socket_));

    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = Start, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
    ASSERT_EQ(A_ERR_MAX, a_Socket_Start(&socket_));
}

TEST_F(Socket, Stop)
{
    EXPECT_CALL(*mock_socket_, Stop(arg_)).Times(1).WillOnce(testing::Return(A_ERR_MAX));

    ASSERT_EQ(A_ERR_NULL, a_Socket_Stop(nullptr));

    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
    ASSERT_EQ(A_ERR_NONE, a_Socket_Stop(&socket_));

    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = Stop, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
    ASSERT_EQ(A_ERR_MAX, a_Socket_Stop(&socket_));
}

TEST_F(Socket, Send)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x00U, 0x03U, 0x04U, 0x05U};
    a_Buffer_t buffer;
    std::uint32_t arg;

    arg_ = &arg;
    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Buffer_SetWrite(&buffer, sizeof(data));
    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));

    EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, &arg)).Times(1).WillOnce(testing::ReturnArg<1>());

    ASSERT_EQ(A_ERR_NULL, a_Socket_Send(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Send(&socket_, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Socket_Send(&socket_, &buffer));
}

TEST_F(Socket, Receive)
{
    std::uint8_t data[6U];
    a_Buffer_t buffer;
    std::uint32_t arg;

    arg_ = &arg;
    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = nullptr, .stop = nullptr, .send = Send, .receive = Receive, .arg = arg_}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));

    EXPECT_CALL(*mock_socket_, Receive(testing::_, testing::_, &arg)).Times(1).WillOnce(testing::Return(0U));

    ASSERT_EQ(A_ERR_NULL, a_Socket_Receive(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Socket_Receive(&socket_, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Socket_Receive(&socket_, &buffer));
}
