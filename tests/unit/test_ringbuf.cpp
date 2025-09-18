/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <array>

#include <gtest/gtest.h>

#include "ringbuf.h"

namespace {

TEST(RingBufferTest, PushAndPopSingleValues) {
    core::RingBuffer buffer(4);

    EXPECT_TRUE(buffer.Push(0x11));
    EXPECT_TRUE(buffer.Push(0x22));
    EXPECT_EQ(buffer.Size(), 2u);

    uint8_t value = 0;
    EXPECT_TRUE(buffer.Pop(value));
    EXPECT_EQ(value, 0x11);
    EXPECT_EQ(buffer.Size(), 1u);

    EXPECT_TRUE(buffer.Pop(value));
    EXPECT_EQ(value, 0x22);
    EXPECT_TRUE(buffer.Empty());
}

TEST(RingBufferTest, PushFailsWhenFull) {
    core::RingBuffer buffer(2);
    EXPECT_TRUE(buffer.Push(1));
    EXPECT_TRUE(buffer.Push(2));
    EXPECT_FALSE(buffer.Push(3));
    EXPECT_TRUE(buffer.Full());
}

TEST(RingBufferTest, WrapAroundWorks) {
    core::RingBuffer buffer(3);

    EXPECT_TRUE(buffer.Push(0xAA));
    EXPECT_TRUE(buffer.Push(0xBB));

    uint8_t value = 0;
    EXPECT_TRUE(buffer.Pop(value));
    EXPECT_EQ(value, 0xAA);

    EXPECT_TRUE(buffer.Push(0xCC));
    EXPECT_TRUE(buffer.Push(0xDD));
    EXPECT_TRUE(buffer.Full());

    std::array<uint8_t, 3> out{};
    EXPECT_EQ(buffer.Pop(out.data(), out.size()), 3u);
    EXPECT_EQ(out[0], 0xBB);
    EXPECT_EQ(out[1], 0xCC);
    EXPECT_EQ(out[2], 0xDD);
    EXPECT_TRUE(buffer.Empty());
}

}  // namespace
