#include <cstdint>
#include <set>

#include <gtest/gtest.h>

#include "random.h"

TEST(Random, Get32)
{
    std::set<std::uint32_t> values;
    a_Random_Seed();

    for (int i = 0; i < 256; i++)
    {
        std::uint32_t value = a_Random_Get32();

        ASSERT_FALSE(values.contains(value));
        values.insert(value);
    }
}
