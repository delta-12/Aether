#include <gmock/gmock.h>

#include "err.h"

class AbstractSocket
{
public:
    virtual a_Err_t Start(void) const = 0;
    virtual a_Err_t Stop(void) const = 0;
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(uint8_t *const data, const size_t size) const = 0;
};

class MockSocket : public AbstractSocket
{
public:
    MOCK_METHOD(a_Err_t, Start, (), (const, override));
    MOCK_METHOD(a_Err_t, Stop, (), (const, override));
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (uint8_t *const data, const size_t size), (const, override));
};
