/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace core {

class RingBuffer {
  public:
    explicit RingBuffer(std::size_t capacity);

    bool Push(uint8_t value);
    std::size_t Push(const uint8_t* data, std::size_t length);

    bool Pop(uint8_t& value);
    std::size_t Pop(uint8_t* dest, std::size_t length);

    bool Peek(uint8_t& value) const;

    void Clear();

    [[nodiscard]] bool Empty() const { return size_ == 0; }
    [[nodiscard]] bool Full() const { return size_ == capacity_; }
    [[nodiscard]] std::size_t Size() const { return size_; }
    [[nodiscard]] std::size_t Capacity() const { return capacity_; }

  private:
    std::size_t Increment(std::size_t index) const;

    std::vector<uint8_t> buffer_;
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
};

}  // namespace core
