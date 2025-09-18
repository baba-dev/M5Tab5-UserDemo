/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace core {

class SettingsStore {
  public:
    SettingsStore();
    explicit SettingsStore(std::unordered_map<std::string, std::string> initial_values);

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;

    template <typename T>
    void Set(const std::string& key, const T& value) {
        values_[key] = ToString(value);
    }

    template <typename T>
    T Get(const std::string& key, const T& default_value) const {
        auto found = values_.find(key);
        if (found == values_.end()) {
            return default_value;
        }
        auto parsed = FromString<T>(found->second);
        if (!parsed.has_value()) {
            return default_value;
        }
        return parsed.value();
    }

    bool Remove(const std::string& key);
    bool Contains(const std::string& key) const;
    void Clear();
    [[nodiscard]] std::size_t Size() const { return values_.size(); }

  private:
    template <typename T>
    static std::string ToString(const T& value);

    template <typename T>
    static std::optional<T> FromString(const std::string& value);

    std::unordered_map<std::string, std::string> values_;
};

// Explicit specialisations

template <>
std::string SettingsStore::ToString<std::string>(const std::string& value);

template <>
std::string SettingsStore::ToString<bool>(const bool& value);

template <>
std::optional<std::string> SettingsStore::FromString<std::string>(const std::string& value);

template <>
std::optional<bool> SettingsStore::FromString<bool>(const std::string& value);

}  // namespace core

#include "settings_impl.h"
