/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

namespace core {

template <typename T>
std::string SettingsStore::ToString(const T& value) {
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

template <typename T>
std::optional<T> SettingsStore::FromString(const std::string& value) {
    std::istringstream stream(value);
    T result;
    if constexpr (std::is_same_v<T, bool>) {
        stream >> std::boolalpha >> result;
    } else {
        stream >> result;
    }
    if (stream.fail()) {
        return std::nullopt;
    }
    return result;
}

}  // namespace core
