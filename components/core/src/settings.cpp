/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "settings.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace core {

namespace {
std::string Trim(std::string value) {
    auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return {};
    }
    auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}
}  // namespace

SettingsStore::SettingsStore() = default;

SettingsStore::SettingsStore(std::unordered_map<std::string, std::string> initial_values)
    : values_(std::move(initial_values)) {}

bool SettingsStore::LoadFromFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return false;
    }

    std::unordered_map<std::string, std::string> parsed;
    std::string line;
    while (std::getline(input, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        std::string key   = Trim(line.substr(0, pos));
        std::string value = Trim(line.substr(pos + 1));
        if (!key.empty()) {
            parsed[key] = value;
        }
    }

    values_ = std::move(parsed);
    return true;
}

bool SettingsStore::SaveToFile(const std::string& path) const {
    std::ofstream output(path, std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }

    for (const auto& [key, value] : values_) {
        output << key << '=' << value << '\n';
    }
    return true;
}

bool SettingsStore::Remove(const std::string& key) {
    return values_.erase(key) > 0U;
}

bool SettingsStore::Contains(const std::string& key) const {
    return values_.find(key) != values_.end();
}

void SettingsStore::Clear() {
    values_.clear();
}

// Explicit specialisations

template <>
std::string SettingsStore::ToString<std::string>(const std::string& value) {
    return value;
}

template <>
std::string SettingsStore::ToString<bool>(const bool& value) {
    return value ? "true" : "false";
}

template <>
std::optional<std::string> SettingsStore::FromString<std::string>(const std::string& value) {
    return value;
}

template <>
std::optional<bool> SettingsStore::FromString<bool>(const std::string& value) {
    std::string lowered;
    lowered.resize(value.size());
    std::transform(value.begin(), value.end(), lowered.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (lowered == "true" || lowered == "1" || lowered == "yes") {
        return true;
    }
    if (lowered == "false" || lowered == "0" || lowered == "no") {
        return false;
    }
    return std::nullopt;
}

}  // namespace core
