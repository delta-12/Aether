#include <cstdint>
#include <span>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "err.h"
#include "hashmap.h"

#define SPAN_FROM_VALUE(value, size) std::span<std::uint8_t>(static_cast<std::uint8_t *>(value), size)

/* TODO deinit hashmap after each test, run with valgrind */

TEST(Hashmap, Initialize)
{
    a_Hashmap_t hashmap;

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Initialize(nullptr, sizeof(std::uint32_t), 10U));

    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Initialize(&hashmap, 0U, 10U));
    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Initialize(&hashmap, sizeof(std::uint32_t), 0U));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, sizeof(std::uint32_t), 10U));
}

TEST(Hashmap, Insert)
{
    a_Hashmap_t hashmap;
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap, sizeof(std::uint32_t), 10U);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(nullptr, &key, value));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, nullptr, value));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, &key, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, value));
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, value));

    value[0U] = 0x10U;
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, value));

    /* TODO test handling collisions and if table is full */
}

TEST(Hashmap, Get)
{
    a_Hashmap_t hashmap;
    std::uint32_t key_0 = 0x12345678U;
    std::uint8_t value_0[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    std::uint32_t key_1 = 0x91011121U;
    std::uint8_t value_1[10U] = {0x10U, 0x11U, 0x12U, 0x13U, 0x14U, 0x15U, 0x16U, 0x17U, 0x18U, 0x19U};
    a_Hashmap_Initialize(&hashmap, sizeof(std::uint32_t), 10U);
    a_Hashmap_Insert(&hashmap, &key_0, value_0);
    a_Hashmap_Insert(&hashmap, &key_1, value_1);

    ASSERT_EQ(nullptr, a_Hashmap_Get(nullptr, &key_0));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, nullptr));

    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_0), sizeof(value_0)), testing::ElementsAreArray(value_0));
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_1), sizeof(value_1)), testing::ElementsAreArray(value_1));

    value_0[0U] = 0x10U;
    value_1[0U] = 0x00U;
    a_Hashmap_Insert(&hashmap, &key_0, value_0);
    a_Hashmap_Insert(&hashmap, &key_1, value_1);
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_0), sizeof(value_0)), testing::ElementsAreArray(value_0));
    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key_1), sizeof(value_1)), testing::ElementsAreArray(value_1));

    /* TODO */
}

TEST(Hashmap, Remove)
{
    a_Hashmap_t hashmap;
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap, sizeof(std::uint32_t), 10U);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(nullptr, &key));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(&hashmap, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key));

    a_Hashmap_Insert(&hashmap, &key, value);
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key));
    ASSERT_EQ(nullptr, a_Hashmap_Get(&hashmap, &key));

    /* TODO */
}
