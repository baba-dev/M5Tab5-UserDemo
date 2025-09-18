/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ringbuf.h"

#include <algorithm>
#include <stdexcept>

namespace core {

RingBuffer::RingBuffer(std::size_t capacity) : buffer_(capacity), capacity_(capacity) {
    if (capacity == 0) {
        throw std::invalid_argument("RingBuffer capacity must be greater than zero");
    }
}

bool RingBuffer::Push(uint8_t value) {
    if (Full()) {
        return false;
    }
    buffer_[head_] = value;
    head_          = Increment(head_);
    ++size_;
    return true;
}

std::size_t RingBuffer::Push(const uint8_t* data, std::size_t length) {
    if (data == nullptr || length == 0) {
        return 0;
    }

    std::size_t written = 0;
    while (written < length && !Full()) {
        if (Push(data[written])) {
            ++written;
        } else {
            break;
        }
    }
    return written;
}

bool RingBuffer::Pop(uint8_t& value) {
    if (Empty()) {
        return false;
    }
    value = buffer_[tail_];
    tail_ = Increment(tail_);
    --size_;
    return true;
}

std::size_t RingBuffer::Pop(uint8_t* dest, std::size_t length) {
    if (dest == nullptr || length == 0) {
        return 0;
    }

    std::size_t read = 0;
    while (read < length && !Empty()) {
        if (Pop(dest[read])) {
            ++read;
        } else {
            break;
        }
    }
    return read;
}

bool RingBuffer::Peek(uint8_t& value) const {
    if (Empty()) {
        return false;
    }
    value = buffer_[tail_];
    return true;
}

void RingBuffer::Clear() {
    head_ = tail_ = size_ = 0;
}

std::size_t RingBuffer::Increment(std::size_t index) const {
    return (index + 1U) % capacity_;
}

}  // namespace core
