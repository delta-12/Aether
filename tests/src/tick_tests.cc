#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "tick.h"

TEST(Tick, GetTick)
{
    a_Tick_Ms_t tick = a_Tick_GetTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NEAR(a_Tick_GetTick(), ++tick, 1U);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NEAR(a_Tick_GetTick(), ++tick, 1U);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NEAR(a_Tick_GetTick(), ++tick, 1U);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_NEAR(a_Tick_GetTick(), ++tick, 1U);
}

TEST(Tick, GetElapsed)
{
    a_Tick_Ms_t tick = a_Tick_GetTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ASSERT_NEAR(5U, a_Tick_GetElapsed(tick), 1U);
}
